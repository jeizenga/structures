// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.


//  immutable_list.hpp
//  
// Contains a template implementation of a Lisp-style linked list with shared
// rest pointers.
//

#ifndef structures_immutable_list_hpp
#define structures_immutable_list_hpp

#include <memory>
#include <mutex>

using namespace std;


/**
 * A class that deletes things in a queue, non-recursively.
 * Destructors may call back to it to enqueue more deletions.
 * Objects to delete may come in from other threads.
 * Loosely based on the approach from <https://stackoverflow.com/a/36635668>.
 *
 * It IS allowed to be owned by something it deletes, as long as that something
 * is the last thing it will ever delete, because it is vary careful about not
 * using its member variables after it does the last deletion.
 *
 */
template<typename T>
class QueueDeleter {
public:
    /// Queue something for deletion
    void queue(T* to_delete);
private:
    /// We internally have a linked list of things to delete.
    /// This looks a bit like the list below but we don't do any fancy magic memory management.
    struct Node {
        T* data;
        Node* next;
    };
    
    /// Head of the linked list of things to delete
    Node* head = nullptr;
    /// Tail of the linked list of things to delete
    Node* tail = nullptr;
    
    /// We have a mutex to protect the queue
    recursive_mutex queue_mutex;
};

/**
 * A Lisp-style singly-linked list.
 * We support O(1) copy and O(1) copy-and-prepend.
 *
 * We avoid the case where a long chain of shared_ptrs to each other can cause
 * a stack overflow on destruction because the destructors are recursively
 * called.
 */
template<typename T>
class ImmutableList {
public:
    /// Make a new empty ImmutableList
    ImmutableList() = default;
    
    /// Make an ImmutableList by prepending a value to an existing one, or onto an empty list.
    ImmutableList(const T& item, const ImmutableList<T>& rest = ImmutableList());
    
    /// Prepend an item and return the list with the item prepended
    ImmutableList<T> push_front(const T& item) const;
    
    /// Get the rest of the list with the first item removed.
    /// The first item must exist.
    ImmutableList<T> pop_front() const;
    
    /// Get the first item, which must exist
    const T& front() const;
    
    /// Decide if the list is empty
    bool empty() const;
    
    /// Compare two lists for less-than (if T supports it)
    bool operator<(const ImmutableList<T>& other) const;
    
    /// Compare two lists for greater-than (if T supports it)
    bool operator>(const ImmutableList<T>& other) const;
    
    /// Compare two lists for equality (if T supports it)
    bool operator==(const ImmutableList<T>& other) const;
    
    /// Compare two lists for inequality (if T supports it)
    bool operator!=(const ImmutableList<T>& other) const;
    
private:
    
    /// We define a Node class; an ImmutableList is just a handle to a shared_ptr-owned head Node.
    struct Node {
        /// Holds the data item for this node, or null if this node represents an empty list.
        const unique_ptr<const T> data;
        /// Holds the next list node, if any
        const shared_ptr<const Node> next;
        /// Holds the deleter in charge of deleting nodes in a queue-based, non-recursive way.
        /// Must not be null.
        /// One of these will be created per terminal list node.
        /// The delete has to be non-const to work.
        const shared_ptr<QueueDeleter<Node>> deleter;
    };
    
public:
    
    /// Provide an iterator that goes from start to end
    struct iterator {
        /// Advance the iterator in place
        iterator& operator++();
        
        /// Check for equality
        bool operator==(const iterator& other);
        
        /// Check for inequality
        bool operator!=(const iterator& other);
        
        /// Get the thing the iterator points to
        const T& operator*() const;
        
        /// Follow this pointer when doing -> on the iterator
        const T* operator->() const;
        
        /// Internally track the node we are on, or nullptr for end
        const Node* here;
    };
    
    /// Get an iterator to the start of the list
    iterator begin() const;
    /// Get an iterator to the past-the-end of the list
    iterator end() const;

private:
    
    /// Get a shared pointer to a new empty node, with a custom deleter that avoids recursive deletion.
    /// Takes the data item, and the optional head node of the list to extend.
    static shared_ptr<const Node> create_node(const T& data,
        shared_ptr<const Node> next = shared_ptr<const Node>());

    /// This holds the head Node for this list, or null for an empty list
    shared_ptr<const Node> head;

};


template<typename T>
void QueueDeleter<T>::queue(T* to_delete) {
    // We need to queue this thing for deletion

    {
        // Lock the queue.
        lock_guard<recursive_mutex> queue_guard(queue_mutex);
        
        // We will set this to true if another thread (or us further up the stack) is processing the queue.
        // As long as the queue is nonempty, somebody (the person to make it nonempty) is processing it.
        bool deleting = (tail != nullptr);
        
        // Append the thing to delete to the list
        Node* added = new Node{to_delete, nullptr};
        if (deleting) {
            // Just put it at the end
            tail->next = added;
            tail = added;
        } else {
            // Make it the entire list
            tail = added;
            head = added;
        }
        
        if (deleting) {
            // Somebody else will take care of it.
            return;
        }
    }
    
    // Otherwise, we are the one responsible for deleting all this stuff, since we made the queue nonempty.
    
    while (true) {
        // We will loop around, locking the queue, popping, and deleting. But
        // we can't put a loop condition because we can't protect it.
        
        // Grab a node for us to delete
        Node* delete_node;
        
        {
            lock_guard<recursive_mutex> queue_guard(queue_mutex);
        
            // Pop a node
            delete_node = head;
            
            if (delete_node == nullptr) {
                // Nothing to do!
                return;
            }
            
            // Otherwise write it out of the queue
            head = delete_node->next;
            if (tail == delete_node) {
                // List is now empty
                tail = nullptr;
            }
        }
        
        // Determine if we are clearing the queue or not. If we are clearing
        // the queue, it's possible we will delete ourselves, so we won't be
        // able to access member variables after the deletion.
        bool queue_clear = (delete_node->next == nullptr);
        
        // Delete the data. Note that this can affect head and tail!
        // Note also that this can delete *us* if this was the last node in the queue!
        delete delete_node->data;
        // Delete the node that carried it.
        delete delete_node;
        
        if (queue_clear) {
            // No more deleting to do! And we might have deleted ourselves!
            return;
        }
    }
}

template<typename T>
ImmutableList<T>::ImmutableList(const T& item, const ImmutableList<T>& rest) : head(create_node(item, rest.head)) {
    // Nothing to do!
}

template<typename T>
auto ImmutableList<T>::push_front(const T& item) const -> ImmutableList<T> {
    return ImmutableList<T>(item, *this);
}

template<typename T>
auto ImmutableList<T>::pop_front() const -> ImmutableList<T> {
    ImmutableList<T> popped;
    popped.head = head->next;
    return popped;
}

template<typename T>
auto ImmutableList<T>::front() const -> const T& {
    return *(head->data);
}

template<typename T>
auto ImmutableList<T>::empty() const -> bool {
    return head.get() == nullptr;
}

// TODO: replace these recursive implementations which might not get tail call optimized!

template<typename T>
auto ImmutableList<T>::operator<(const ImmutableList<T>& other) const -> bool {
    if (empty()) {
        // Empty is < filled
        return !other.empty();
    }
    if (other.empty()) {
        return false;
    }
    
    if (front() < other.front()) {
        // We're smaller
        return true;
    } else if (other.front() < front()) {
        // They're smaller
        return false;
    } else {
        // Equal fronts means compare the rests.
        return pop_front() < other.pop_front();
    }
}

template<typename T>
auto ImmutableList<T>::operator>(const ImmutableList<T>& other) const -> bool {
    if (other.empty()) {
        // Filled is > empty
        return !empty();
    }
    if (empty()) {
        return false;
    }
    
    if (other.front() > front()) {
        // They're larger
        return false;
    } else if (front() > other.front()) {
        // We're larger
        return true;
    } else {
        // Equal fronts means compare the rests.
        return pop_front() > other.pop_front();
    }
}

template<typename T>
auto ImmutableList<T>::operator==(const ImmutableList<T>& other) const -> bool {
    if (empty()) {
        // Empties are equal
        return other.empty();
    }
    if (other.empty()) {
        return false;
    }
    
    if (front() == other.front()) {
        // Keep looking
        return pop_front() == other.pop_front();
    } else {
        return false;
    }
}

template<typename T>
auto ImmutableList<T>::operator!=(const ImmutableList<T>& other) const -> bool {
    if (empty()) {
        // Empties are equal
        return !other.empty();
    }
    if (other.empty()) {
        return true;
    }
    
    if (front() != other.front()) {
        // We found a difference
        return true;
    } else {
        // Keep looking
        return pop_front() != other.pop_front();
    }
}

template<typename T>
auto ImmutableList<T>::iterator::operator++() -> iterator& {
    here = here->next.get();
    return *this;
}

template<typename T>
auto ImmutableList<T>::iterator::operator==(const iterator& other) -> bool {
    return here == other.here;
}

template<typename T>
auto ImmutableList<T>::iterator::operator!=(const iterator& other) -> bool {
    return here != other.here;
}

template<typename T>
auto ImmutableList<T>::iterator::operator*() const -> const T& {
    return *(here->data);
}

template<typename T>
auto ImmutableList<T>::iterator::operator->() const -> const T* {
    return here->data.get();
}

template<typename T>
auto ImmutableList<T>::begin() const -> iterator {
    return iterator{head.get()};
}

template<typename T>
auto ImmutableList<T>::end() const -> iterator {
    return iterator{nullptr};
}

template<typename T>
auto ImmutableList<T>::create_node(const T& data, shared_ptr<const Node> next) -> shared_ptr<const Node> {
    // We create a new node pointing to the next node, if any, and return a shared pointer for it.
    
    // Find or create the deleter
    shared_ptr<QueueDeleter<Node>> deleter;
    if (next.get() != nullptr) {
        // A deleter exists. Grab a reference.
        deleter = next->deleter;
    } else {
        // Make a new deleter for this new tail node.
        deleter = make_shared<QueueDeleter<Node>>();
    }
    
    // Make the actual node
    Node* created = new Node{unique_ptr<const T>(new T(data)), next, deleter};
    
    // Give ownership to a shared pointer that will delete it with the deleter, and return it
    return shared_ptr<const Node>(created, [](Node* to_delete) {
        to_delete->deleter->queue(to_delete);
    });
}




#endif /* structures_immutable_list_hpp */

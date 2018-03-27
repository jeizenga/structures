// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
//                                               "License"); you may not use this file except in compliance
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


//  updateable_priority_queue.hpp
//  
// Contains a template implementation of a priority queue with updateable priorities.

#ifndef structures_updateable_priority_queue_hpp
#define structures_updateable_priority_queue_hpp

#include <queue>
#include <unordered_set>

namespace structures {

using namespace std;

/**
 * A priority queue where priorities can be updated by inserting the same
 * element again with a different priority. Since std::priority_queue just uses
 * element comparison for priority, we have another template for an identity
 * type, and a function to extract it from an element, so we can see if the
 * same element (with a different priority) has come out yet.
 */
template<
    class T,
    class Identity,
    class Container = std::vector<T>,
    class Compare = std::less<typename Container::value_type>
>
class UpdateablePriorityQueue {
public:
    /// Make a UpdateablePriorityQueue that uses the inserted objects as their own identities.
    /// This requires that hash values and equality checks for the item ignore priority.
    UpdateablePriorityQueue();
    /// Make a UpdateablePriorityQueue that uses the given function to determine element identity.
    UpdateablePriorityQueue(const std::function<Identity(const T&)>& get_identity);
    
    // We can support all the priority queue functions except size because we don't know how many things are redundant.
    const T& top() const;
    void pop();
    bool empty() const;
    void push(const T& item);
    
    template<class... Args>
    void emplace(Args&&... args);
    
    /// Clear the heap and all its memories of past elements.
    void clear();
    
private:
    /// The actual underlying priority queue
    priority_queue<T, Container, Compare> queue;
    /// The set to deduplicate results.
    unordered_set<Identity> seen;
    /// The element extractor
    std::function<Identity(const T&)> get_identity;
};





template<class T, class Identity, class Container, class Compare>
UpdateablePriorityQueue<T, Identity, Container, Compare>::UpdateablePriorityQueue() :
    UpdateablePriorityQueue((std::function<Identity(const T&)>)([](const T& item) -> Identity { return item; })) {
    
    static_assert(std::is_same<T, Identity>::value, "can only use the identity identity function if items are their own Identitiies");
}

template<class T, class Identity, class Container, class Compare>
UpdateablePriorityQueue<T, Identity, Container, Compare>::UpdateablePriorityQueue(
    const std::function<Identity(const T&)>& get_identity) : get_identity(get_identity) {
    
    // Nothing to do!
    
}

template<class T, class Identity, class Container, class Compare>
auto UpdateablePriorityQueue<T, Identity, Container, Compare>::top() const -> const T& {
    assert(!seen.count(get_identity(queue.top())));
    return queue.top();
}

template<class T, class Identity, class Container, class Compare>
void UpdateablePriorityQueue<T, Identity, Container, Compare>::pop() {
    // We need to pop off the top thing, mark it as seen, and remove any other
    // copies of it so we maintain the invariant that the thing at the top is
    // new.
    seen.insert(get_identity(top()));
    queue.pop();
    while (!empty() && seen.count(get_identity(queue.top()))) {
        queue.pop();
    }
}

template<class T, class Identity, class Container, class Compare>
bool UpdateablePriorityQueue<T, Identity, Container, Compare>::empty() const {
    return queue.empty();
}

template<class T, class Identity, class Container, class Compare>
void UpdateablePriorityQueue<T, Identity, Container, Compare>::push(const T& item) {
    if (!seen.count(get_identity(item))) {
        // This is new, so queue it
        queue.push(item);
    }
}

template<class T, class Identity, class Container, class Compare>
template<class... Args>
void UpdateablePriorityQueue<T, Identity, Container, Compare>::emplace(Args&&... args) {
    // To get the benefit of emplace we always add the thing to the queue
    queue.emplace(std::forward<Args>(args)...);
    
    while (!empty() && seen.count(get_identity(queue.top()))) {
        // And then we clean up any invariant-violating repeats on top of the queue
        queue.pop();
    }
}

template<class T, class Identity, class Container, class Compare>
void UpdateablePriorityQueue<T, Identity, Container, Compare>::clear() {
    queue = priority_queue<T, Container, Compare>();
    seen.clear();
}


}

#endif

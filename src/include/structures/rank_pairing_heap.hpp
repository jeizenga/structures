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


//  rank_pairing_heap.hpp
//
// Contains an implementation of the rank-pairing heap described in Haeupler, et al. (2011).
//

#ifndef structures_rank_pairing_heap_hpp
#define structures_rank_pairing_heap_hpp

#include <forward_list>
#include <vector>
#include <cstdlib>

using namespace std;

/**
 * A priority queue data structure that allows amortized O(1) priority increases.
 * Each value is only allows to be popped one time. Values must also be hashable.
 */
template <typename T, typename PriorityType, typename Compare = less<PriorityType>>
class RankPairingHeap {
public:
    
    /// Construct an empty heap.
    RankPairingHeap();
    
    /// Construct an empty heap using a non-default comparator.
    RankPairingHeap(const Compare& compare);
    
    /// Destructor.
    ~RankPairingHeap();
    
    /// Return the highest priority item on the heap and its priority.
    inline const pair<T, PriorityType>& top() const;
    
    /// Add the value to the heap if it has not been added yet. If it has been added
    /// but has not been popped, set its priority to the maximum of the current priority
    /// and the given priority. If it has been popped already, do nothing.
    inline void push_or_reprioritize(const T& value, const PriorityType& priority);
    
    /// Remove the highest priority item from the heap.
    inline void pop();
    
    /// Return true if there are no items in the heap, else false.
    inline void empty() const;
    
    /// Return the number of items on the heap.
    size_t size() const;
    

private:
    
    class Node;
    
    /// Add a half-tree to the primary tree through the tournament procedure.
    inline void place_half_tree(Node* node);
    
    /// Link two half-tree roots and update rank.
    inline void link(Node* winner, Node* loser);
    
    /// Give an item the maximum of the given priority and its current priority.
    inline void reprioritize(Node* node, const PriorityType& priority);
    
    /// Find the nodes in the heap by value
    unordered_map<T, Node*> current_nodes;
    
    /// Root of the tree
    forward_list<Node*> roots;
    
    /// Tracker to enable size query
    size_t num_items = 0;
    
    /// Comparator we are using to select maximum
    Compare compare;
};







/*
 * Represents a value in the heap and a node in the binary tree structure.
 */
template <typename T, typename PriorityType, typename Compare>
class RankPairingHeap<T, PriorityType, Compare>::Node {
public:
    Node(const T& value, const PriorityType& priority) value(value, priority) {}
    ~Node() {
        delete left;
        delete right;
    }
    pair<T, PriorityType> value;
    
    uint64_t rank = 0;
    Node* parent = nullptr;
    Node* left = nullptr;
    Node* right = nullptr;
}

template <typename T, typename PriorityType, typename Compare>
RankPairingHeap<T, PriorityType, Compare>::RankPairingHeap() {
    // nothing to do
}

template <typename T, typename PriorityType, typename Compare>
RankPairingHeap<T, PriorityType, Compare>::RankPairingHeap(const Compare& compare) compare(compare) {
    // nothing to do
}

template <typename T, typename PriorityType, typename Compare>
RankPairingHeap<T, PriorityType, Compare>::~RankPairingHeap() {
    // clean up the heap tree
    delete root;
}

template <typename T, typename PriorityType, typename Compare>
void RankPairingHeap<T, PriorityType, Compare>::link(Node* winner, Node* loser) {
    // tied contests increase the winner's rank
    if (winner->rank == loser->rank) {
        winner->rank++;
    }
    // place winner's left subtree on loser's right subtree
    loser->right = winner->left
    if (loser->right) {
        loser->right->parent = loser;
    }
    // winner's left subtree is now the loser
    winner->left = loser;
    loser->parent = winner;
}

template <typename T, typename PriorityType, typename Compare>
void RankPairingHeap<T, PriorityType, Compare>::place_half_tree(Node* node) {
    
    if (roots.empty()) {
        // this is the first value
        roots.push_front(node);
    }
    else if (compare(top().second, node->value.second)) {
        // this is the new maximum
        roots.push_front(node);
    }
    else {
        // this is not the new maximum
        roots.insert_after(roots.begin(), node);
    }
}

template <typename T, typename PriorityType, typename Compare>
inline void RankPairingHeap<T, PriorityType, Compare>::push_or_reprioritize(const T& value, const PriorityType& priority) {
    
    // look for the value in the heap
    auto current_location = current_nodes.find(value);
    if (current_location != current_nodes.end()) {
        // we've seen this value before
        if (current_location->second) {
            // it hasn't been popped yet, give it the new priority
            reprioritize(node, priority);
        }
    }
    else {
        // we haven't seen this value before, make a new node
        Node* node = new Node(value, priority);
        
        // add it to the heap
        place_half_tree(node);
        
        // bookkeeping
        current_nodes[value] = node;
        num_items++;
    }
}

template <typename T, typename PriorityType, typename Compare>
inline const pair<T, PriorityType>& RankPairingHeap<T, PriorityType, Compare>::top() const {
    return roots.front()->value;
}

template <typename T, typename PriorityType, typename Compare>
inline void RankPairingHeap<T, PriorityType, Compare>::reprioritize(Node* node, const PriorityType& priority) {
    
    if (compare(node->value.second, priority)) {
        // we're giving it a higher priority than it currently has
        if (!node->parent) {
            // we don't need to rearrange this tree because it's already the highest priority element
            node->value.second = priority;
            
            if (compare(top().second, priority)) {
                // this is now the highest priority root, so we need to move it to the front
                // TODO: how do I efficiently find this root in the list?
                // could switch to one Node* and an unordered_set<Node*> ...
                auto prev_iter = roots.begin();
                auto iter = prev_iter;
                ++iter;
                for (; iter != roots.end(); ++iter, ++prev_iter) {
                    if (*iter == node) {
                        roots.erase_after(prev_iter);
                        roots.push_front(node);
                        break;
                    }
                }
            }
        }
        else {
            // remove this node from the tree and put its right subtree in its place
            Node* next_parent = node->parent;
            node->parent = nullptr
            node->right->parent = next_parent;
            if (next_parent->left == node) {
                next_parent->left = node->right;
            }
            else {
                next_parent->right = node->right;
            }
            
            place_half_tree(node);
            
            // restore the rank property above this parent
            while (next_parent) {
                if (next_parent == root) {
                    next_parent->rank = next_parent->left->rank + 1;
                }
                else {
                    // this is an internal parent, make it a (1,1), (1, 2), or, (0, i) node
                    uint64_t next_rank = max(next_parent->left->rank, next_parent->right->rank);
                    if (next_rank - min(next_parent->left->rank, next_parent->right->rank) <= 1) {
                        next_rank++;
                    }
                    if (next_rank >= next_parent->rank) {
                        break;
                    }
                    else {
                        next_parent->rank = next_rank;
                    }
                }
                next_parent = next_parent->parent;
            }
        }
    }
}

template <typename T, typename PriorityType, typename Compare>
inline void RankPairingHeap<T, PriorityType, Compare>::pop() {
    
    // bookkeeping
    num_items--;
    // mark this value as popped
    current_nodes[top().first] = nullptr;
    
    // remove the current first root
    Node* first_root = roots.front();
    roots.pop_front();
    
    // collect the other roots for later processing
    forward_list<Node*> new_roots;
    new_roots.swap(roots);
    
    // disassemble the first tree and collect the right spine
    if (first_root->left) {
        
        // have to have one iteration outside the loop since we travel
        // left the first time
        Node* prev_spine_node = first_root->left;
        new_roots.push_front(prev_spine_node);
        prev_spine_node->parent = nullptr;
        while (prev_spine_node->right) {

            new_roots.push_front(prev_spine_node->right);
            prev_spine_node->right = nullptr;
            
            prev_spine_node = new_roots.front();
            prev_spine_node->parent = nullptr;
            
        }
    }
    
    // get rid of the root
    first_root->left = nullptr; // so it won't delete the other nodes
    delete first_root;
    
    // one-pass algorithm over the roots described in paper
    vector<Node*> buckets;
    for (Node* half_tree_root : spine) {
        
        // compact the ranks to make 1-nodes
        half_tree_root->rank = half_tree_root->left ? half_tree_root->left->rank + 1 : 0;
        
        // ensure that we have enough buckets
        while (buckets.size() + 1 < half_tree_root->rank) {
            buckets.push_back(nullptr);
        }
        
        // what's in the bucket right now?
        Node* other_root = buckets[half_tree_root->rank];
        if (other_root) {
            // there's already a tree in this bucket
            // TODO: is it actually necessary to compare at this stage?
            if (compare(half_tree_root->value.second, other_root->value.second)) {
                // the current tree wins, link and place it
                link(other_root, half_tree_root);
                place_half_tree(other_root);
            }
            else {
                // the other tree wins, link and place it
                link(half_tree_root, other_root);
                place_half_tree(half_tree_root);
            }
            // empty the bucket
            buckets[half_tree_root->rank] = nullptr;
        }
        else {
            // the bucket is empty, fill it with the current half tree
            buckets[half_tree_root->rank] = half_tree_root;
        }
    }
    
    // get any half trees still in the buckets and add them to the main tree
    for (Node* half_tree_root : buckets) {
        if (half_tree_root) {
            place_half_tree(half_tree_root);
        }
    }
}

template <typename T, typename PriorityType, typename Compare>
inline bool RankPairingHeap<T, PriorityType, Compare>::empty() const {
    return roots.empty();
}

template <typename T, typename PriorityType, typename Compare>
inline size_t RankPairingHeap<T, PriorityType, Compare>::size() const {
    return num_items;
}


#endif /* structures_rank_pairing_heap_hpp */

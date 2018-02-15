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


//  min_max_heap.hpp
//  
// Contains a template implementation of a min-max heap
//

#ifndef structures_min_max_heap_hpp
#define structures_min_max_heap_hpp

#include <vector>
#include <cstdint>

using namespace std;


/*
 * A dynamic container structure that supports efficient maximum and minimum operations.
 */
template <typename T>
class MinMaxHeap {
public:
    
    /// Initialize a heap with the values returned by an iterator in linear time
    template<typename Iter>
    MinMaxHeap(Iter begin, Iter end);
    
    /// Initialize an empty heap
    MinMaxHeap();
    
    /// Add a value to the heap in logarithmic time
    inline void push(const T& value);
    
    /// Construct a value on the heap in place in logarithmic time
    template<typename... Args>
    inline void emplace(Args&&... args);
    
    /// Returns the maximum value of the heap in constant time
    inline const T& max();
    
    /// Returns the minimum value of the heap in constant time
    inline const T& min();
    
    /// Remove the maximum element of the heap in logarithmic time
    inline void pop_max();
    
    /// Remove the minimum element of the heap in logarithmic time
    inline void pop_min();
    
    /// Returns true if the heap contains no values, else false
    inline bool empty();
    
    /// Returns the number of values in the heap
    inline size_t size();
    
private:
    
    inline void post_add();
    void restore_heap_below(size_t i, int level);
    void restore_heap_above(size_t i, int level);
    inline static bool cmp(const T& v1, const T& v2, int level);
    
    vector<T> values;
};













template <typename T>
MinMaxHeap<T>::MinMaxHeap() {
    // nothing to do
}

template <typename T>
template <typename Iter>
MinMaxHeap<T>::MinMaxHeap(Iter begin, Iter end) {
    for (auto iter = begin; iter != end; iter++) {
        values.push_back(*iter);
    }
    
    if (values.empty()) {
        return;
    }
    
    // depth of the current layer of internal nodes
    int level = -2;
    // size at which we would begin filling the next level of the tree
    size_t next_level_begin = 1;
    while (next_level_begin - 1 < values.size()) {
        next_level_begin *= 2;
        level++;
    }
    
    // the range of indices that correspond to the deepest internal layer
    size_t internal_level_end = next_level_begin / 2 - 1;
    size_t internal_level_begin = next_level_begin / 4 - 1;
    
    // set up the heap invariant from the deepest layer up to the root
    while (level >= 0) {
        for (size_t i = internal_level_begin; i < internal_level_end; i++) {
            restore_heap_below(i, level);
        }
        
        internal_level_end = internal_level_begin;
        internal_level_begin = (internal_level_begin + 1) / 2 - 1;
        level--;
    }
}

template <typename T>
inline bool MinMaxHeap<T>::cmp(const T& v1, const T& v2, int level) {
    return (level % 2 == 0) != (v1 > v2);
}

template <typename T>
void MinMaxHeap<T>::restore_heap_below(size_t i, int level) {
    // the range of i's grandchildren
    size_t rightest = 4 * i + 6;
    size_t leftest = rightest - 3;
    if (leftest >= values.size()) {
        // i has no grandchildren
        size_t left = 2 * i + 1;
        size_t right = left + 1;
        if (left >= values.size()) {
            // i has no children
            return;
        }
        // find the extremal element among the children
        size_t most = cmp(values[i], values[left], level) ? i : left;
        if (right < values.size()) {
            most = cmp(values[most], values[right], level) ? most : right;
        }
        
        // if necessary swap, no need to recurse further because only one level
        // below (invariant vacuously maintained)
        if (most != i) {
            swap(values[i], values[most]);
        }
    }
    else {
        // find the extremal element among the grandchildren
        size_t most = cmp(values[i], values[leftest], level) ? i : leftest;
        for (size_t j = leftest + 1; j <= rightest; j++) {
            if (j >= values.size()) {
                break;
            }
            most = cmp(values[most], values[j], level) ? most : j;
        }
        
        bool direct_child_swapped = false;
        if (leftest + 2 >= values.size()) {
            // the right child has no children, so we may need to swap with it directly
            size_t right = 2 * i + 2;
            if (cmp(values[right], values[most], level)) {
                swap(values[i], values[right]);
                direct_child_swapped = true;
            }
        }
        
        // if we swapped, and not with child (because then it has nogrand children and
        // invariant is vacuously maintained below) then recurse downward
        if (!direct_child_swapped && most != i) {
            swap(values[i], values[most]);
            size_t intermediate = most <= leftest + 1 ? 2 * i + 1 : 2 * i + 2;
            if (cmp(values[most], values[intermediate], level + 1)) {
                swap(values[intermediate], values[most]);
            }
            
            restore_heap_below(most, level + 2);
        }
    }
}

template <typename T>
void MinMaxHeap<T>::restore_heap_above(size_t i, int level) {
    if (i <= 2) {
        // i has no grandparent
        return;
    }
    // go two layers up to get the next value using the same direction of comparison
    size_t grandparent = (i + 1) / 4 - 1;
    if (cmp(values[i], values[grandparent], level - 2)) {
        // swap and recurse upward
        swap(values[i], values[grandparent]);
        restore_heap_above(grandparent, level - 2);
    }
}

template <typename T>
inline const T& MinMaxHeap<T>::min() {
    assert(!values.empty());
    return values[0];
}

template <typename T>
inline const T& MinMaxHeap<T>::max() {
    assert(!values.empty());
    if (values.size() == 1) {
        return values[0];
    }
    else if (values.size() == 2) {
        return values[1];
    }
    else {
        return std::max(values[1], values[2]);
    }
}

template <typename T>
inline void MinMaxHeap<T>::push(const T& value) {
    values.push_back(value);
    post_add();
}

template <typename T>
template <typename... Args>
inline void MinMaxHeap<T>::emplace(Args&&... args) {
    values.emplace_back(std::forward<Args>(args)...);
    post_add();
}

template <typename T>
inline void MinMaxHeap<T>::post_add() {
    if (values.size() == 1) {
        // no parents to restore invariants in
        return;
    }
    
    int64_t i = values.size() - 1;
    size_t parent = (i + 1) / 2 - 1;
    
    // depth of the current layer of leaves
    int level = -1;
    // size at which we would begin filling the next level of the tree
    size_t next_level_begin = 1;
    while (next_level_begin - 1 < values.size()) {
        next_level_begin *= 2;
        level++;
    }
    
    // decide whether this value should go in the min or max layers and then recurse upward
    if (cmp(values[i], values[parent], level - 1)) {
        swap(values[i], values[parent]);
        restore_heap_above(parent, level - 1);
    }
    else {
        restore_heap_above(i, level);
    }
}

template <typename T>
inline void MinMaxHeap<T>::pop_min() {
    assert(!values.empty());
    // move the back value into the  minimum value's position and then
    // restore the invariant
    values.front() = values.back();
    values.pop_back();
    restore_heap_below(0, 0);
}

template <typename T>
inline void MinMaxHeap<T>::pop_max() {
    assert(!values.empty());
    if (values.size() <= 2) {
        // the max is either the only element or the only element in
        // a max layer
        values.pop_back();
    }
    else  {
        // get the index of the max value
        size_t i = values[1] > values[2] ? 1 : 2;
        // move the value at the back into this position and then restore
        // the invariant
        values[i] = values.back();
        values.pop_back();
        restore_heap_below(i, 1);
    }
}

template <typename T>
inline size_t MinMaxHeap<T>::size() {
    return values.size();
}

template <typename T>
inline bool MinMaxHeap<T>::empty() {
    return values.empty();
}

#endif /* structures_min_max_heap_hpp */

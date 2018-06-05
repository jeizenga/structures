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


//  range_min_query.hpp
//
//

#ifndef structures_range_min_query_hpp
#define structures_range_min_query_hpp

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <deque>

using namespace std;

/**
 *
 */
template <class RandomAccessIterator>
class RangeMinQuery {
public:
    /// Construct a RangeMinQuery for the half-open interval between these iterators. Any operation that
    /// invalidates the iterators in this interval also invalidates the RangeMinQuery.
    RangeMinQuery(RandomAccessIterator begin, RandomAccessIterator end);
    
    /// Returns an iterator to the minimum-valued element in half-open interval between these iterators. The
    /// ends of this interval must be between the ends of the interval used to construct the RangeMinQuery.
    RandomAccessIterator range_min(RandomAccessIterator range_begin, RandomAccessIterator range_end);
    
private:
    
    class CartesianTree;
    class IntervalMemo;
    struct Block;
    
    RandomAccessIterator begin;
    RandomAccessIterator end;
    
    size_t block_size;
    vector<Block> blocks;
    
    vector<IntervalMemo> cartesian_tree_memos;
    
    vector<size_t> greatest_smaller_power_of_2;
    vector<size_t> powers_of_2;
    vector<vector<size_t>> power_block_range_memo;
};




struct RangeMinQuery<RandomAccessIterator>::Block {
public:
    Block(RandomAccessIterator begin, RandomAccessIterator end,
          size_t cartestian_tree_index) : begin(begin), end(end), cartestian_tree_index(cartestian_tree_index) {}
    const size_t cartestian_tree_index;
    const RandomAccessIterator begin;
    const RandomAccessIterator end;
};




class RangeMinQuery<RandomAccessIterator>::CartesianTree {
    
    CartesianTree(RandomAccessIterator begin, RandomAccessIterator end);
    ~CartesianTree();
    
    uint64_t bit_encoding();
    IntervalMemo interval_memo();
    size_t size();
    
private:
    struct Node;
    
    size_t num_nodes;
    
    Node* root;
};

RangeMinQuery<RandomAccessIterator>::CartesianTree::CartesianTree(RandomAccessIterator begin, RandomAccessIterator end) {
    
    assert(begin <= end);
    
    num_nodes = (end - begin);
    
    if (begin == end) {
        root = nullptr;
        return;
    }
    
    RandomAccessIterator iter = begin;
    root = new Node(iter);
    
    Node* prev = root;
    for (; iter != end; iter++) {
        Node* new_node = new Node(iter);
        
        Node* here = prev;
        while (here ? *(new_node->iter) < *(here->iter) : false) {
            here = here->parent;
        }
        
        if (here) {
            new_node->parent = here;
            new_node->left = here->right;
            here->right = new_node;
        }
        else {
            new_node->left = root;
            root = new_node;
        }
        
        prev = new_node;
    }
}

RangeMinQuery<RandomAccessIterator>::CartesianTree::~CartesianTree() {
    delete root;
}

RangeMinQuery<RandomAccessIterator>::CartesianTree::size() {
    return num_nodes;
}

RangeMinQuery<RandomAccessIterator>::CartesianTree::bit_encoding() {
    
    deque<Node*> queue{root};
    
    uint64_t encoding = 0;
    
    size_t i = 0;
    while (!queue.empty()) {
        Node* node = queue.front();
        if (node) {
            encoding |= (1 << i);
            queue.push_back(node->left);
            queue.push_back(node->right);
        }
        i++;
    }
    return encoding;
}

struct RangeMinQuery<RandomAccessIterator>::CartesianTree::Node {
    
    Node(RandomAccessIterator iter) iter(iter), parent(nullptr), left(nullptr), right(nullptr) {}
    ~Node() {
        delete left;
        delete right;
    }
    
    RandomAccessIterator iter;
    Node* parent;
    Node* left;
    Node* right;
};


RangeMinQuery<RandomAccessIterator>::RangeMinQuery(RandomAccessIterator begin, RandomAccessIterator end) : begin(begin), end(end) {
    
    assert(end >= begin);
    
    int64_t size = end - begin;
    int64_t log_size = 0;
    while (int64_t tmp = size; tmp > 1; tmp /= 2) {
        log_size++;
    }
    block_size = log_size / 4 + 1;
    
    powers_of_2.reserve(log_size);
    greatest_smaller_power_of_2.resize(size, 0);
    size_t power_of_2 = 1;
    size_t power = 0;
    size_t next_power_of_2 = 2;
    powers_of_2.push_back(power_of_2);
    for (size_t i = 1; i < greatest_smaller_power_of_2.size(); i++) {
        if (i == next_power_of_2) {
            power_of_2 = next_power_of_2;
            powers_of_2.push_back(power_of_2);
            next_power_of_2 *= 2;
            power++;
        }
        greatest_smaller_power_of_2[i] = power;
    }
    
    
    blocks.reserve((size - 1) / block_size + 1);
    unordered_map<uint64_t, size_t> tree_id_to_index;
    for (auto block_begin = begin; block_begin < end; block_begin += block_size) {
        auto block_end = block_begin + block_size;
        if (block_end > end) {
            block_end = end;
        }
        
        CartesianTree cartesian_tree(block_begin, block_end);
        uint64_t tree_id = cartesian_tree.bit_encoding();
        size_t tree_idx;
        if (tree_id_to_index.count(tree_id)) {
            tree_idx = tree_id_to_index[tree_id];
        }
        else {
            tree_idx = cartesian_tree_memos.size();
            tree_id_to_index[tree_id] = tree_idx;
            cartesian_tree_memos.emplace_back(cartesian_tree.interval_memo());
        }
        
        blocks.emplace_back(block_begin, block_end, tree_idx);
    }
    
    power_block_range_memo.resize(blocks.size());
    for (size_t i = 0; i + 1 < blocks.size(); i++) {
        Block& block = blocks[i];
        power_block_range_memo[i].reserve(greatest_smaller_power_of_2[blocks.size() - i]);
        power_block_range_memo[i].push_back(cartesian_tree_memos[block.cartestian_tree_index].range_min_index(0, block.end - block.begin));
    }
    
    for (size_t j = 1; j < powers_of_2.size(); j++) {
        size_t power_of_2 = powers_of_2[j];
        for (size_t i = 0; i + power_of_2 < blocks.size(); i++) {
            size_t lower = power_block_range_memo[i][j - 1];
            size_t upper = power_block_range_memo[i + power_of_2][j - 1];
            power_block_range_memo[i].push_back(*(begin + lower) < *(begin + upper) ? lower : upper);
        }
    }
}

RandomAccessIterator RangeMinQuery<RandomAccessIterator>::range_min(RandomAccessIterator range_begin, RandomAccessIterator range_end) {
    
    assert(range_begin >= begin && range_end <= end && range_begin < range_end);
    
    size_t begin_block_idx = (range_begin - begin) / block_size;
    size_t end_block_idx = (range_end - begin - 1) / block_size;
    
    RandomAccessIterator iter;
    if (begin_block_idx == end_block_idx) {
        // they are in the same block, so we can use the Cartesian tree memo directly
        Block& block = blocks[begin_block_idx];
        size_t relative_begin = range_begin - block.begin;
        size_t relative_end = range_end - block.begin;
        iter = block.begin + cartesian_tree_memos[block.cartestian_tree_index].range_min_index(relative_begin, relative_end);
    }
    else {
        // they are in separate blocks, so we have to use the Cartesian tree memos on each end
        Block& begin_block = blocks[begin_block_idx];
        Block& end_block = blocks[end_block_idx];
        
        size_t relative_begin = range_begin - begin_block.begin;
        size_t relative_end = range_end - end_block.begin;
        
        auto begin_iter = begin_block.begin + cartesian_tree_memos[block.cartestian_tree_index].range_min_index(relative_begin, begin_block.end - begin_block.begin);
        auto end_iter = end_block.begin + cartesian_tree_memos[block.cartestian_tree_index].range_min_index(0, relative_end);
        
        iter = *begin_iter < *end_iter ? begin_iter : end_iter;
        
        if (begin_block_idx + 1 < end_block_idx) {
            // there are blocks in the middle, so we have to use whole-block range-based search
            size_t power_interval = greatest_smaller_power_of_2[end_block_idx - begin_block_idx - 1];
            auto lower = begin + power_block_range_memo[begin_block_idx + 1][power_interval];
            auto upper = begin + power_block_range_memo[end_block_idx - powers_of_2[power_interval]][power_interval];
            if (*lower < *iter) {
                iter = lower;
            }
            if (*upper < *iter) {
                iter = lower;
            }
        }
    }
    return iter;
}





#endif /* structures_range_min_query_hpp */

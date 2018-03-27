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

#include "structures/union_find.hpp"

namespace structures {

using namespace std;

struct UnionFind::UFNode {
    UFNode(size_t index) : rank(0), size(1), head(index) {}
    ~UFNode() {}
    
    size_t rank;
    size_t size;
    size_t head;
    unordered_set<size_t> children;
};

UnionFind::UnionFind(size_t size) {
    uf_nodes.reserve(size);
    for (size_t i = 0; i < size; i++) {
        uf_nodes.emplace_back(i);
    }
}

UnionFind::~UnionFind() {
    // nothing to do
}

size_t UnionFind::size() {
    return uf_nodes.size();
}

size_t UnionFind::find_group(size_t i) {
    vector<size_t> path;
    // traverse tree upwards
    while (uf_nodes[i].head != i) {
        path.push_back(i);
        i = uf_nodes[i].head;
    }
    // compress path
    unordered_set<size_t>& head_children = uf_nodes[i].children;
    for (size_t p = 1; p < path.size(); p++) {
        size_t j = path[p - 1];
        uf_nodes[j].head = i;
        uf_nodes[path[p]].children.erase(j);
        head_children.insert(j);
    }
    // note: don't need to compress path for the final index since it
    // already points to the head
    return i;
}

void UnionFind::union_groups(size_t i, size_t j) {
    size_t head_i = find_group(i);
    size_t head_j = find_group(j);
    if (head_i == head_j) {
        // the indices are already in the same group
        return;
    }
    else {
        // use rank as a pivot to determine which group to make the head
        UFNode& node_i = uf_nodes[head_i];
        UFNode& node_j = uf_nodes[head_j];
        if (node_i.rank > node_j.rank) {
            node_j.head = head_i;
            node_i.children.insert(head_j);
            node_i.size += node_j.size;
        }
        else {
            node_i.head = head_j;
            node_j.children.insert(head_i);
            node_j.size += node_i.size;
            
            if (node_j.rank == node_i.rank) {
                node_j.rank++;
            }
        }
    }
}

size_t UnionFind::group_size(size_t i) {
    return uf_nodes[find_group(i)].size;
}

vector<size_t> UnionFind::group(size_t i) {
    vector<size_t> to_return;
    // go to head of group
    vector<size_t> stack{find_group(i)};
    // traverse tree downwards to find all indices in group
    while (!stack.empty()) {
        size_t curr = stack.back();
        stack.pop_back();
        to_return.push_back(curr);
        unordered_set<size_t>& children = uf_nodes[curr].children;
        for (size_t child : children) {
            stack.push_back(child);
        }
    }
    return to_return;
}

vector<vector<size_t>> UnionFind::all_groups() {
    vector<vector<size_t>> to_return(uf_nodes.size());
    for (size_t i = 0; i < uf_nodes.size(); i++) {
        to_return[find_group(i)].push_back(i);
    }
    auto new_end = std::remove_if(to_return.begin(), to_return.end(),
                                  [](const vector<size_t>& grp) { return grp.empty(); });
    to_return.resize(new_end - to_return.begin());
    return to_return;
}

}

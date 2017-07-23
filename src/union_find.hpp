//
//  union_find.hpp
//
// Contains an implementation of a custom union-find with some additional capabilities
//

#ifndef union_find_hpp
#define union_find_hpp

#include <vector>
#include <unordered_set>
#include <cstdint>
#include <algorithm>

using namespace std;

/**
 * A custom Union-Find data structure that supports merging a set of indices in
 * disjoint sets in amortized nearly linear time. This implementation also supports
 * querying the size of the group containing an index in constant time and querying
 * the members of the group containing an index in linear time in the size of the group.
 */
class UnionFind {
public:
    /// Construct UnionFind for this many indices
    UnionFind(size_t size);
    
    /// Destructor
    ~UnionFind();
    
    /// Returns the number of indices in the UnionFind
    size_t size();
    
    /// Returns the group ID that index i belongs to (can change after calling union)
    size_t find_group(size_t i);
    
    /// Merges the group containing index i with the group containing index j
    void union_groups(size_t i, size_t j);
    
    /// Returns the size of the group containing index i
    size_t group_size(size_t i);
    
    /// Returns a vector of the indices in the same group as index i
    vector<size_t> group(size_t i);
    
    /// Returns all of the groups, each in a separate vector
    vector<vector<size_t>> all_groups();
    
private:
    
    struct UFNode;
    vector<UFNode> uf_nodes;
};












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



#endif /* union_find_hpp */

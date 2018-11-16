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


//  suffix_tree.hpp
//  
// Suffix tree implementation including Ukkonen's linear time construction algorithm
//


#ifndef structures_suffix_tree_hpp
#define structures_suffix_tree_hpp

#include <unordered_map>
#include <list>
#include <string>
#include <vector>
#include <cstdint>

namespace structures {

using namespace std;

/**
 * An implementation of a suffix tree with linear time and space complexity for construction.
 */
class SuffixTree {
    
public:
    /// Linear time constructor.
    ///
    /// Note: string cannot have null characters, but this is not checked.
    SuffixTree(string::const_iterator begin, string::const_iterator end);
    ~SuffixTree() = default;
    
    /// Returns the length of the longest prefix of str that exactly matches
    /// a suffix of the string used to construct the suffix tree.
    ///
    /// Note: string cannot have null characters, but this is not checked.
    size_t longest_overlap(const string& str);
    
    /// Same semantics as previous
    size_t longest_overlap(string::const_iterator begin, string::const_iterator end);
    
    /// Retuns a vector of all of the indices where a string occurs as a substring
    /// of the string used to construct the suffix tree. Indices are ordered arbitrarily.
    ///
    /// Note: string cannot have null characters, but this is not checked.
    vector<size_t> substring_locations(const string& str);
    vector<size_t> substring_locations(string::const_iterator begin, string::const_iterator end);
    
    /// Beginning of string used to make tree
    const string::const_iterator begin;
    
    /// End of string used to make tree
    const string::const_iterator end;
    
private:
    struct STNode;
    
    /// All nodes in the tree (in a list to avoid difficulties with pointers and reallocations)
    list<STNode> nodes;
    
    /// The edges from the root node
    unordered_map<char, STNode*> root;
    
    /// Returns a char of the string or null char at past-the-last index
    inline char get_char(size_t i);
};


/**
 * A node of a suffix tree corresponding a substring of the string
 */
struct SuffixTree::STNode {
    /// Constructor
    STNode(int64_t first, int64_t last);
    ~STNode() = default;
    
    /// Edges down the tree
    unordered_map<char, STNode*> children;
    
    /// First index of string on this node
    int64_t first;
    /// Last index of string on this node, inclusive (-1 indicates end sentinel during consruction)
    int64_t last;
    
    /// The length of the the node during a phase of construction
    inline int64_t length(int64_t phase) {
        return last >= 0 ? last - first + 1 : phase - first + 1;
    }
    
    /// The last index contained on the this node during a phase of construction
    inline int64_t final_index(int64_t phase) {
        return last >= 0 ? last - first : phase - first;
    }
};

inline char SuffixTree::get_char(size_t i) {
    return i == end - begin ? '\0' : *(begin + i);
}


}



#endif /* structures_suffix_tree_hpp */

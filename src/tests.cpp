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


//  tests.cpp
//
// Unit tests for data structures in this repository
//

#include <stdio.h>
#include <list>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <cassert>

#include "structures/suffix_tree.hpp"
#include "structures/union_find.hpp"
#include "structures/min_max_heap.hpp"

using namespace std;

size_t longest_overlap(string& str1, string& str2) {
    size_t max_possible = min(str1.size(), str2.size());
    
    for (size_t i = str1.size() - max_possible; i < str1.size(); i++) {
        bool match = true;
        for (size_t j = i; j < str1.size(); j++) {
            if (str1[j] != str2[j - i]) {
                match = false;
                break;
            }
        }
        if (match) {
            return str1.size() - i;
        }
    }
    return 0;
}

vector<size_t> substring_locations(string& str, string& substr) {
    
    vector<size_t> locations;
    
    if (substr.empty()) {
        return locations;
    }
    
    for (size_t i = 0; i <= str.size() - substr.size(); i++) {
        bool match = true;
        for (size_t j = 0; j < substr.size(); j++) {
            if (str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            locations.push_back(i);
        }
    }
    return locations;
}

string random_string(string& alphabet, size_t length) {
    
    random_device rd;
    default_random_engine gen(rd());
    uniform_int_distribution<int> distr(0, alphabet.size() - 1);
    
    string str;
    str.reserve(length);
    for (int i = 0; i < length; i++) {
        str.push_back(alphabet[distr(gen)]);
    }
    
    return str;
}

string random_repetive_string(vector<string>& chunks, size_t chunk_length,
                              string& alphabet, double mismatch_rate) {
    
    random_device rd;
    default_random_engine gen(rd());
    uniform_int_distribution<int> char_distr(0, alphabet.size() - 1);
    uniform_int_distribution<int> chunk_distr(0, chunks.size() - 1);
    uniform_real_distribution<double> err_distr(0.0, 1.0);
    
    string str;
    for (int i = 0; i < chunk_length; i++) {
        string chunk = chunks[chunk_distr(gen)];
        for (int j = 0; j < chunk.size(); j++) {
            if (err_distr(gen) < mismatch_rate) {
                chunk[j] = alphabet[char_distr(gen)];
            }
        }
        str.append(chunk);
    }
    return str;
}

string random_substring(string& str, size_t substring_length, string& alphabet, double mismatch_rate) {
    
    if (str.size() < substring_length) {
        return "";
    }
    
    random_device rd;
    default_random_engine gen(rd());
    uniform_int_distribution<int> start_distr(0, str.size() - substring_length);
    uniform_int_distribution<int> char_distr(0, alphabet.size() - 1);
    uniform_real_distribution<double> err_distr(0.0, 1.0);
    
    string substr;
    substr.reserve(substring_length);
    
    int substr_start = start_distr(gen);
    for (int i = substr_start; i < substr_start + substring_length; i++) {
        if (err_distr(gen) < mismatch_rate) {
            substr.push_back(alphabet[char_distr(gen)]);
        }
        else {
            substr.push_back(str[i]);
        }
    }
    return substr;
}

void test_suffix_tree_with_curated_examples() {
    {
        
        string seq = "ACGTGACA";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        assert(suffix_tree.longest_overlap("ACAGCCT") == 3);
    }
    {
        
        string seq = "AATGGCATTNCGNAAGTACAGTG";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        assert(suffix_tree.longest_overlap(seq) == seq.size());
    }
    {
        
        string seq = "AATGGCATTNCGNAAGTACAGTG";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        assert(suffix_tree.longest_overlap("") == 0);
    }
    {
        
        string seq = "";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        assert(suffix_tree.longest_overlap("AATGGCATTNCGNAAGTACAGTG") == 0);
    }
    {
        
        string seq = "";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        assert(suffix_tree.longest_overlap("") == 0);
    }
    {
        
        string seq = "AGTGCGATAGATGATAGAAGATCGCTCGCTCCGCGATA";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        vector<size_t> locs = suffix_tree.substring_locations("GATA");
        sort(locs.begin(), locs.end());
        
        vector<size_t> correct_locs {5, 12, 34};
        
        assert(locs == correct_locs);
    }
    {
        
        string seq = "TACGGCAGATG";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        vector<size_t> locs = suffix_tree.substring_locations("GATA");
        sort(locs.begin(), locs.end());
        
        vector<size_t> correct_locs;
        
        assert(locs == correct_locs);
    }
    {
        
        string seq = "TACGGCAGATG";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        vector<size_t> locs = suffix_tree.substring_locations("TACGGCAGATG");
        sort(locs.begin(), locs.end());
        
        vector<size_t> correct_locs {0};
        
        assert(locs == correct_locs);
    }
    {
        
        string seq = "TACGGCAGATG";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        vector<size_t> locs = suffix_tree.substring_locations("");
        sort(locs.begin(), locs.end());
        
        vector<size_t> correct_locs;
        
        assert(locs == correct_locs);
    }
    {
        
        string seq = "TACGGCAGATG";
        
        SuffixTree suffix_tree(seq.begin(), seq.end());
        
        vector<size_t> locs = suffix_tree.substring_locations("TACGGCAGATGA");
        sort(locs.begin(), locs.end());
        
        vector<size_t> correct_locs;
        
        assert(locs == correct_locs);
    }
    
    cerr << "All curated SuffixTree tests successful!" << endl;
}

void test_suffix_tree_with_randomized_examples() {
    {
        int max_str_len = 30;
        int num_seqs = 1000;
        
        random_device rd;
        default_random_engine gen(rd());
        uniform_int_distribution<int> len_distr(0, max_str_len);
        
        string alphabet = "ACGTN";
        
        for (int i = 0; i < num_seqs; i++) {
            string str1 = random_string(alphabet, len_distr(gen));
            string str2 = random_string(alphabet, len_distr(gen));
            
            SuffixTree suffix_tree(str1.begin(), str1.end());
            
            size_t suffix_tree_longest_overlap = suffix_tree.longest_overlap(str2);
            size_t brute_longest_overlap = longest_overlap(str1, str2);
            
            if (suffix_tree_longest_overlap != brute_longest_overlap) {
                // print out the failures since their random and we might have a hard time finding them again
                cerr << "FAILURE: wrong overlap of " << suffix_tree_longest_overlap << " on " << str1 << " " << str2 << endl;
            }
            
            assert(suffix_tree_longest_overlap == brute_longest_overlap);
        }
    }
    {
        int max_num_chunks = 10;
        int max_chunk_len = 10;
        int num_chunks = 2;
        int num_seqs = 1000;
        double mismatch_rate = .01;
        
        random_device rd;
        default_random_engine gen(rd());
        uniform_int_distribution<int> len_distr(0, max_chunk_len);
        uniform_int_distribution<int> num_chunks_distr(0, max_num_chunks);
        
        
        string alphabet = "ACGTN";
        
        for (int i = 0; i < num_seqs; i++) {
            
            vector<string> chunks;
            for (int j = 0; j < num_chunks; j++) {
                chunks.push_back(random_string(alphabet, len_distr(gen)));
            }
            
            string str1 = random_repetive_string(chunks, num_chunks_distr(gen), alphabet, mismatch_rate);
            string str2 = random_repetive_string(chunks, num_chunks_distr(gen), alphabet, mismatch_rate);
            
            SuffixTree suffix_tree(str1.begin(), str1.end());
            
            size_t suffix_tree_longest_overlap = suffix_tree.longest_overlap(str2);
            size_t brute_longest_overlap = longest_overlap(str1, str2);
            
            if (suffix_tree_longest_overlap != brute_longest_overlap) {
                // print out the failures since their random and we might have a hard time finding them again
                cerr << "FAILURE: wrong overlap on " << str1 << " " << str2 << endl;
            }
            
            assert(suffix_tree_longest_overlap == brute_longest_overlap);
        }
    }
    {
        int num_suffix_trees = 100;
        int num_substrings_per_tree = 10;
        int min_suffix_tree_length = 100;
        int max_suffix_tree_length = 300;
        int max_substring_length = 40;
        double mismatch_rate = .03;
        
        string alphabet = "ACGTN";
        
        random_device rd;
        default_random_engine gen(rd());
        uniform_int_distribution<int> str_len_distr(min_suffix_tree_length, max_suffix_tree_length);
        uniform_int_distribution<int> substr_len_distr(0, max_substring_length);
        
        for (int i = 0; i < num_suffix_trees; i++) {
            
            string str = random_string(alphabet, substr_len_distr(gen));
            
            SuffixTree suffix_tree(str.begin(), str.end());
            
            for (int j = 0; j < num_substrings_per_tree; j++) {
                
                string substr = random_substring(str, substr_len_distr(gen), alphabet, mismatch_rate);
                
                vector<size_t> st_locations = suffix_tree.substring_locations(substr);
                sort(st_locations.begin(), st_locations.end());
                vector<size_t> direct_locations = substring_locations(str, substr);
                
                if (st_locations != direct_locations) {
                    // print out the failures since their random and we might have a hard time finding them again
                    cerr << "FAILURE: wrong substring locations on " << str << " " << substr << endl;
                }
                
                assert(st_locations == direct_locations);
            }
        }
        {
            int num_suffix_trees = 100;
            int num_substrings_per_tree = 10;
            int chunk_bank_size = 3;
            int min_num_chunks = 3;
            int max_num_chunks = 10;
            int min_chunk_length = 5;
            int max_chunk_length = 50;
            int max_substring_length = 40;
            double chunk_mismatch_rate = .05;
            double substring_mismatch_rate = .02;
            
            string alphabet = "ACGTN";
            
            random_device rd;
            default_random_engine gen(rd());
            uniform_int_distribution<int> num_chunks_distr(min_num_chunks, max_num_chunks);
            uniform_int_distribution<int> chunk_len_distr(min_chunk_length, max_chunk_length);
            uniform_int_distribution<int> substr_len_distr(0, max_substring_length);
            
            for (int i = 0; i < num_suffix_trees; i++) {
                
                vector<string> chunks;
                for (int j = 0; j < chunk_bank_size; j++) {
                    chunks.push_back(random_string(alphabet, chunk_len_distr(gen)));
                }
                
                string str = random_repetive_string(chunks, num_chunks_distr(gen), alphabet, chunk_mismatch_rate);
                
                SuffixTree suffix_tree(str.begin(), str.end());
                
                for (int j = 0; j < num_substrings_per_tree; j++) {
                    
                    string substr = random_substring(str, substr_len_distr(gen), alphabet, substring_mismatch_rate);
                    
                    vector<size_t> st_locations = suffix_tree.substring_locations(substr);
                    sort(st_locations.begin(), st_locations.end());
                    vector<size_t> direct_locations = substring_locations(str, substr);
                    
                    if (st_locations != direct_locations) {
                        // print out the failures since their random and we might have a hard time finding them again
                        cerr << "FAILURE: wrong substring locations on " << str << " " << substr << endl;
                    }
                    
                    assert(st_locations == direct_locations);
                }
            }
        }
    }
    
    cerr << "All randomized SuffixTree tests successful!" << endl;
}

vector<pair<size_t, size_t>> random_unions(size_t size) {
    
    int num_pairs = size * size;
    
    random_device rd;
    default_random_engine gen(rd());
    uniform_int_distribution<int> distr(0, num_pairs);
    
    vector<pair<size_t, size_t>> pairs;
    vector<size_t> all_options(num_pairs);
    
    for (size_t i = 0; i < num_pairs; i++) {
        all_options[i] = i;
    }
    
    std::shuffle(all_options.begin(), all_options.end(), gen);
    
    int num_pairs_to_select = distr(gen);
    
    for (int i = 0; i < num_pairs_to_select; i++) {
        pairs.emplace_back(all_options[i] / size, all_options[i] % size);
    }
    
    return pairs;
}

void test_union_find_with_curated_examples() {
    {
        UnionFind union_find(10);
        assert(union_find.find_group(0) != union_find.find_group(1));
        
        assert(union_find.group_size(0) == 1);
        assert(union_find.group_size(1) == 1);
        
        union_find.union_groups(0, 1);
        
        assert(union_find.find_group(0) == union_find.find_group(1));
        
        assert(union_find.group_size(0) == 2);
        assert(union_find.group_size(1) == 2);
    }
    
    {
        UnionFind union_find(10);
        union_find.union_groups(0, 1);
        union_find.union_groups(2, 3);
        union_find.union_groups(3, 4);
        union_find.union_groups(5, 6);
        
        assert(union_find.group_size(4) == 3);
        assert(union_find.find_group(2) == union_find.find_group(3));
        assert(union_find.find_group(5) == union_find.find_group(6));
    }
    
    {
        UnionFind union_find(10);
        union_find.union_groups(0, 1);
        union_find.union_groups(2, 3);
        union_find.union_groups(3, 4);
        union_find.union_groups(5, 6);
        union_find.union_groups(2, 4);
        
        assert(union_find.group_size(4) == 3);
        assert(union_find.find_group(2) == union_find.find_group(3));
        assert(union_find.find_group(3) == union_find.find_group(4));
    }
    
    {
        UnionFind union_find(10);
        union_find.union_groups(0, 1);
        union_find.union_groups(2, 3);
        union_find.union_groups(3, 4);
        union_find.union_groups(5, 6);
        union_find.union_groups(2, 4);
        
        vector<size_t> correct_group{2, 3, 4};
        vector<size_t> group = union_find.group(3);
        std::sort(group.begin(), group.end());
        assert(group.size() == correct_group.size());
        assert(std::equal(group.begin(), group.end(), correct_group.begin()));
    }
    
    {
        UnionFind union_find_1(10);
        UnionFind union_find_2(10);
        
        union_find_1.union_groups(0, 1);
        union_find_1.union_groups(2, 1);
        union_find_1.union_groups(3, 2);
        union_find_1.union_groups(4, 5);
        union_find_1.union_groups(7, 6);
        union_find_1.union_groups(7, 8);
        union_find_1.union_groups(7, 9);
        
        union_find_2.union_groups(0, 1);
        union_find_2.union_groups(2, 1);
        union_find_2.union_groups(3, 2);
        union_find_2.union_groups(4, 5);
        union_find_2.union_groups(7, 6);
        union_find_2.union_groups(7, 8);
        union_find_2.union_groups(7, 9);
        
        vector<size_t> group_of_0_1 = union_find_1.group(0);
        vector<size_t> group_of_4_1 = union_find_1.group(4);
        vector<size_t> group_of_9_1 = union_find_1.group(9);
        
        vector<vector<size_t>> all_groups_1 = union_find_1.all_groups();
        
        vector<vector<size_t>> all_groups_2 = union_find_2.all_groups();
        
        vector<size_t> group_of_0_2 = union_find_2.group(0);
        vector<size_t> group_of_4_2 = union_find_2.group(4);
        vector<size_t> group_of_9_2 = union_find_2.group(9);
        
        vector<size_t> group_of_0_1_from_all, group_of_4_1_from_all, group_of_9_1_from_all;
        vector<size_t> group_of_0_2_from_all, group_of_4_2_from_all, group_of_9_2_from_all;
        
        for (vector<size_t>& vec : all_groups_1) {
            for (size_t i : vec) {
                if (i == 0) {
                    group_of_0_1_from_all = vec;
                }
                else if (i == 4) {
                    group_of_4_1_from_all = vec;
                }
                else if (i == 9) {
                    group_of_9_1_from_all = vec;
                }
            }
        }
        
        for (vector<size_t>& vec : all_groups_2) {
            for (size_t i : vec) {
                if (i == 0) {
                    group_of_0_2_from_all = vec;
                }
                else if (i == 4) {
                    group_of_4_2_from_all = vec;
                }
                else if (i == 9) {
                    group_of_9_2_from_all = vec;
                }
            }
        }
        
        std::sort(group_of_0_1.begin(), group_of_0_1.end());
        std::sort(group_of_4_1.begin(), group_of_4_1.end());
        std::sort(group_of_9_1.begin(), group_of_9_1.end());
        
        std::sort(group_of_0_2.begin(), group_of_0_2.end());
        std::sort(group_of_4_2.begin(), group_of_4_2.end());
        std::sort(group_of_9_2.begin(), group_of_9_2.end());
        
        std::sort(group_of_0_1_from_all.begin(), group_of_0_1_from_all.end());
        std::sort(group_of_4_1_from_all.begin(), group_of_4_1_from_all.end());
        std::sort(group_of_9_1_from_all.begin(), group_of_9_1_from_all.end());
        
        std::sort(group_of_0_2_from_all.begin(), group_of_0_2_from_all.end());
        std::sort(group_of_4_2_from_all.begin(), group_of_4_2_from_all.end());
        std::sort(group_of_9_2_from_all.begin(), group_of_9_2_from_all.end());
        
        assert(group_of_0_1.size() == group_of_0_2.size());
        assert(group_of_0_1.size() == group_of_0_1_from_all.size());
        assert(group_of_0_1.size() == group_of_0_2_from_all.size());
        
        assert(group_of_4_1.size() == group_of_4_2.size());
        assert(group_of_4_1.size() == group_of_4_1_from_all.size());
        assert(group_of_4_1.size() == group_of_4_2_from_all.size());
        
        assert(group_of_9_1.size() == group_of_9_2.size());
        assert(group_of_9_1.size() == group_of_9_1_from_all.size());
        assert(group_of_9_1.size() == group_of_9_2_from_all.size());
        
        assert(std::equal(group_of_0_1.begin(), group_of_0_1.end(), group_of_0_1_from_all.begin()));
        assert(std::equal(group_of_0_1.begin(), group_of_0_1.end(), group_of_0_2_from_all.begin()));
        assert(std::equal(group_of_0_1.begin(), group_of_0_1.end(), group_of_0_2.begin()));
        
        assert(std::equal(group_of_4_1.begin(), group_of_4_1.end(), group_of_4_1_from_all.begin()));
        assert(std::equal(group_of_4_1.begin(), group_of_4_1.end(), group_of_4_2_from_all.begin()));
        assert(std::equal(group_of_4_1.begin(), group_of_4_1.end(), group_of_4_2.begin()));
        
        assert(std::equal(group_of_9_1.begin(), group_of_9_1.end(), group_of_9_1_from_all.begin()));
        assert(std::equal(group_of_9_1.begin(), group_of_9_1.end(), group_of_9_2_from_all.begin()));
        assert(std::equal(group_of_9_1.begin(), group_of_9_1.end(), group_of_9_2.begin()));
    }
    
    {
        UnionFind union_find_1(10);
        UnionFind union_find_2(10);
        
        union_find_1.union_groups(0, 1);
        union_find_1.union_groups(2, 1);
        union_find_1.union_groups(3, 2);
        union_find_1.union_groups(4, 5);
        union_find_1.union_groups(7, 6);
        union_find_1.union_groups(7, 8);
        union_find_1.union_groups(7, 9);
        
        union_find_2.union_groups(0, 1);
        union_find_2.union_groups(2, 1);
        union_find_2.union_groups(3, 2);
        union_find_2.union_groups(4, 5);
        union_find_2.union_groups(7, 6);
        union_find_2.union_groups(7, 8);
        union_find_2.union_groups(7, 9);
        
        assert(union_find_1.group_size(0) == union_find_2.group(0).size());
        assert(union_find_2.group_size(0) == union_find_1.group(0).size());
        assert(union_find_1.group_size(0) == union_find_1.group(0).size());
        assert(union_find_2.group_size(0) == union_find_2.group(0).size());
        
        assert(union_find_1.group_size(3) == union_find_2.group(3).size());
        assert(union_find_2.group_size(3) == union_find_1.group(3).size());
        assert(union_find_1.group_size(3) == union_find_1.group(3).size());
        assert(union_find_2.group_size(3) == union_find_2.group(3).size());
        
        assert(union_find_1.group_size(9) == union_find_2.group(9).size());
        assert(union_find_2.group_size(9) == union_find_1.group(9).size());
        assert(union_find_1.group_size(9) == union_find_1.group(9).size());
        assert(union_find_2.group_size(9) == union_find_2.group(9).size());
    }
    
    cerr << "All curated UnionFind tests successful!" << endl;
}

void test_union_find_with_random_examples() {
    for (size_t repetition = 0; repetition < 1000; repetition++) {
        
        UnionFind union_find(30);
        
        vector<pair<size_t, size_t>> unions = random_unions(union_find.size());
        
        vector<unordered_set<size_t>*> group_set_of(union_find.size());
        vector<unordered_set<size_t>> group_sets(union_find.size());
        for (size_t i = 0; i < union_find.size(); i++) {
            group_set_of[i] = &group_sets[i];
            group_sets[i].insert(i);
        }
        
        for (pair<size_t, size_t> idxs : unions) {
            union_find.union_groups(idxs.first, idxs.second);
            
            auto group_set_1 = group_set_of[idxs.first];
            auto group_set_2 = group_set_of[idxs.second];
            if (group_set_1 == group_set_2) {
                continue;
            }
            for (size_t i : *group_set_2) {
                group_set_1->insert(i);
                group_set_of[i] = group_set_1;
            }
            group_set_2->clear();
        }
        
        vector<vector<size_t>> groups_direct(union_find.size());
        vector<vector<size_t>> groups_from_all(union_find.size());
        vector<vector<size_t>> groups_orthogonal(union_find.size());
        // alternate the order these operations are done in just in case the
        // internal changes during find functions affects the outcome
        if (repetition % 2 == 0) {
            for (size_t i = 0; i < union_find.size(); i++) {
                groups_direct[i] = union_find.group(i);
            }
            
            vector<vector<size_t>> groups = union_find.all_groups();
            for (vector<size_t>& group : groups) {
                for (size_t i : group) {
                    groups_from_all[i] = group;
                }
            }
        }
        else {
            vector<vector<size_t>> groups = union_find.all_groups();
            for (vector<size_t>& group : groups) {
                for (size_t i : group) {
                    groups_from_all[i] = group;
                }
            }
            
            for (size_t i = 0; i < union_find.size(); i++) {
                groups_direct[i] = union_find.group(i);
            }
        }
        
        for (size_t i = 0; i < union_find.size(); i++) {
            vector<size_t>& group = groups_orthogonal[i];
            for (size_t j : *group_set_of[i]) {
                group.push_back(j);
            }
        }
        
        for (size_t i = 0; i < union_find.size(); i++) {
            
            std::sort(groups_orthogonal[i].begin(), groups_orthogonal[i].end());
            std::sort(groups_from_all[i].begin(), groups_from_all[i].end());
            std::sort(groups_direct[i].begin(), groups_direct[i].end());
            
            
            assert(union_find.group_size(i) == groups_from_all[i].size());
            assert(union_find.group_size(i) == groups_direct[i].size());
            assert(groups_from_all[i].size() == groups_direct[i].size());
            assert(groups_orthogonal[i].size() == groups_direct[i].size());
            
            if (!std::equal(groups_from_all[i].begin(), groups_from_all[i].end(),
                            groups_direct[i].begin())) {
                cerr << "FAILURE mismatch in repetition " << repetition << " in the following groups containing " << i << endl;
                cerr << "computed directly: ";
                for (size_t j : groups_direct[i]) {
                    cerr << j << " ";
                }
                cerr << endl;
                cerr << "computed in batch: ";
                for (size_t j : groups_from_all[i]) {
                    cerr << j << " ";
                }
                cerr << endl;
                cerr << "groups formed by unions: ";
                for (pair<size_t, size_t> idxs : unions) {
                    cerr << "(" << idxs.first << "," << idxs.second << ") ";
                }
                cerr << endl;
            }
            
            assert(std::equal(groups_from_all[i].begin(), groups_from_all[i].end(),
                               groups_direct[i].begin()));
            assert(std::equal(groups_from_all[i].begin(), groups_from_all[i].end(),
                               groups_orthogonal[i].begin()));
        }
    }
    
    cerr << "All randomized UnionFind tests successful!" << endl;
}

void check_heap_invariants(MinMaxHeap<int>& heap, list<int>& vals) {
    
    if (heap.size() != vals.size()) {
        cerr << "size from heap " << heap.size() << " mismatches direct size " << vals.size() << endl;
        assert(0);
    }
    
    if (heap.empty()) {
        return;
    }
    
    if (heap.max() != *max_element(vals.begin(), vals.end())) {
        cerr << "max from heap " << heap.max() << " mismatches direct max " << *max_element(vals.begin(), vals.end()) << endl;
        assert(0);
    }
    
    if (heap.min() != *min_element(vals.begin(), vals.end())) {
        cerr << "min from heap " << heap.min() << " mismatches direct min " << *min_element(vals.begin(), vals.end()) << endl;
        assert(0);
    }
}

void test_min_max_heap() {
    int num_repetitions = 10000;
    int heapify_min_size = 0;
    int heapify_max_size = 32;
    int max_size = 64;
    int check_frequency = 5;
    
    random_device rd;
    default_random_engine gen(rd());
    uniform_int_distribution<int> distr(numeric_limits<int>::min(),
                                        numeric_limits<int>::max());
    for (int repetition = 0; repetition < num_repetitions; repetition++) {
        
        list<int> vals;
        
        int heapify_size = distr(gen);
        heapify_size = heapify_size < 0 ? -heapify_size : heapify_size;
        heapify_size = heapify_min_size + (heapify_size % (heapify_max_size - heapify_min_size + 1));
        
        for (int i = 0; i < heapify_size; i++) {
            vals.push_back(distr(gen));
        }
        MinMaxHeap<int> heap(vals.begin(), vals.end());
        
        check_heap_invariants(heap, vals);
        
        for (int i = heapify_size; i < max_size; i++) {
            int next = distr(gen);
            vals.push_back(next);
            heap.push(next);
            if (i % check_frequency == 0) {
                check_heap_invariants(heap, vals);
            }
        }
        
        for (int i = 0; i < max_size; i++) {
            if (distr(gen) % 2 == 0) {
                heap.pop_max();
                vals.erase(max_element(vals.begin(), vals.end()));
            }
            else {
                heap.pop_min();
                vals.erase(min_element(vals.begin(), vals.end()));
            }
            if (i % check_frequency == 0) {
                check_heap_invariants(heap, vals);
            }
        }
        
        assert(heap.empty());
        
        MinMaxHeap<int> fresh_heap;
        
        for (int i = 0; i < max_size; i++) {
            int next = distr(gen);
            vals.push_back(next);
            fresh_heap.emplace(next);
            if (i % check_frequency == 0) {
                check_heap_invariants(fresh_heap, vals);
            }
        }
        
        for (int i = 0; i < max_size; i++) {
            if (distr(gen) % 2 == 0) {
                fresh_heap.pop_max();
                vals.erase(max_element(vals.begin(), vals.end()));
            }
            else {
                fresh_heap.pop_min();
                vals.erase(min_element(vals.begin(), vals.end()));
            }
            if (i % check_frequency == 0) {
                check_heap_invariants(fresh_heap, vals);
            }
        }
        
        assert(heap.empty());
    }
    
    cerr << "All MinMaxHeap tests successful!" << endl;
}

int main(void) {
    
    test_min_max_heap();
    test_union_find_with_curated_examples();
    test_union_find_with_random_examples();
    test_suffix_tree_with_curated_examples();
    test_suffix_tree_with_randomized_examples();
}

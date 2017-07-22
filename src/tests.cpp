//
//  tests.cpp
//
//

#include <stdio.h>
#include <list>
#include <iostream>
#include <algorithm>
#include <random>
#include <cassert>


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
            fresh_heap.push(next);
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
}

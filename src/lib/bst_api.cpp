#include "AVL.h"
#include "RedBlack.h"
#include "Splay.h"
#include "BTree.h"
#include <ctime>
#include <cstdlib>
#include <iostream>

extern "C" {
    // 枚举操作类型
    enum class Operation {
        INSERT,
        SEARCH,
        REMOVE,
        LOCALITY
    };

    double benchmark(Operation op, int scale, int method) {
        Vector<int> vec;
        for (int i = 0; i < scale; i++)
            vec.insert(i);
        vec.unsort();
        sleep_seconds(1); // Windows 无 sleep，使用跨平台封装

        AVL<int> avl;
        RedBlack<int> rb;
        Splay<int> splay;
        BTree<int> btree;

        clock_t start, end;

        switch (op) {
            case Operation::INSERT:
                start = clock();
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.insert(vec[i]); break;
                        case 1: rb.insert(vec[i]); break;
                        case 2: splay.insert(vec[i]); break;
                        case 3: btree.insert(vec[i]); break;
                    }
                }
                end = clock();
                break;
            case Operation::SEARCH:
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.insert(vec[i]); break;
                        case 1: rb.insert(vec[i]); break;
                        case 2: splay.insert(vec[i]); break;
                        case 3: btree.insert(vec[i]); break;
                    }
                }
                start = clock();
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.search(dice(scale)); break;
                        case 1: rb.search(dice(scale)); break;
                        case 2: splay.search(dice(scale)); break;
                        case 3: btree.search(dice(scale)); break;
                    }
                }
                end = clock();
                break;
            
            case Operation::REMOVE:
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.insert(vec[i]); break;
                        case 1: rb.insert(vec[i]); break;
                        case 2: splay.insert(vec[i]); break;
                        case 3: btree.insert(vec[i]); break;
                    }
                }
                start = clock();
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.remove(vec[i]); break;
                        case 1: rb.remove(vec[i]); break;
                        case 2: splay.remove(vec[i]); break;
                        case 3: btree.remove(vec[i]); break;
                    }
                }
                end = clock();
                break;
            
            case Operation::LOCALITY:
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.insert(vec[i]); break;
                        case 1: rb.insert(vec[i]); break;
                        case 2: splay.insert(vec[i]); break;
                        case 3: btree.insert(vec[i]); break;
                    }
                }
                int locality = std::max(1, scale / 1000);
                start = clock();
                for (int i = 0; i < scale; i++) {
                    switch (method) {
                        case 0: avl.search(i % locality); break;
                        case 1: rb.search(i % locality); break;
                        case 2: splay.search(i % locality); break;
                        case 3: btree.search(i % locality); break;
                    }
                }
                end = clock();
                break;
        }
        
        return double(end - start) / CLOCKS_PER_SEC;
    }
}

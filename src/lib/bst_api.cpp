#include "AVL.h"
#include "RedBlack.h"
#include "Splay.h"
#include "BTree.h"
#include <ctime>
#include <cstdlib>
#include <iostream>

extern "C" {
    double test_insert(int scale, int method) {
        Vector<int> vec;
        for(int i = 0; i < scale; i++)
            vec.insert(i);
        vec.unsort();
        clock_t start, end;

        start = clock();
        switch(method){
            case 0: {
                AVL<int> avl;
                for(int i = 0; i < scale; i++)
                    avl.insert(vec[i]);
                break;
            }
            case 1: {
                RedBlack<int> rb;
                for(int i = 0; i < scale; i++)
                    rb.insert(vec[i]);
                break;
            }
            case 2: {
                Splay<int> splay;
                for(int i = 0; i < scale; i++)
                    splay.insert(vec[i]);
                break;
            }
            case 3: {
                BTree<int> btree;
                for(int i = 0; i < scale; i++)
                    btree.insert(vec[i]);
                break;
            }
            default:
                return -1;
        }
        end = clock();

        return double(end - start) / CLOCKS_PER_SEC;
    }
}

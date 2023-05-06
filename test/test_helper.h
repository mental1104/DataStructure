#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <coroutine>

#include "BinTree.h"
#include "utils.h"

using namespace::std;

// 生成一棵二叉树
template <typename T>
BinTree<T> generateBinTree() {
    BinTree<int> bintree;
    bintree.insertAsRoot(dice(NUM));
    BinNode<int>* node = bintree.root();
    for(int i = 0; i < 10; i++){
        bintree.insertAsLC(node, dice(NUM));
        bintree.insertAsRC(node, dice(NUM));
        if(dice(NUM) % 2)
            node = node->lc;
        else
            node = node->rc;
    }
    return bintree;
}

// 二叉树中序递归遍历
template<typename T>
void traverse(BinNode<T>* node, std::ostream& os) {
    if(node == nullptr)
        return;
    traverse(node->lc, os);
    os << node->data << " ";
    traverse(node->rc, os);
}
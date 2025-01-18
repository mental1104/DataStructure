/*
 * @Author: mental1104 mental1104@gmail.com
 * @Date: 2023-05-06 21:33:19
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-15 23:02:20
 * @FilePath: /espeon/code/DataStructure/test/test_helper.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include "BinTree.h"
#include "utils.h"

using namespace::std;

/*
 * 二叉树相关辅助函数
 *
 */



/*
 * 生成一颗二叉树
 */
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

/*
 * 二叉树中序递归遍历，用于测试
 */
template<typename T>
void traverse(BinNode<T>* node, std::ostream& os) {
    if(node == nullptr)
        return;
    traverse(node->lc, os);
    os << node->data << " ";
    traverse(node->rc, os);
}
/*
 * @Author: mental1104 mental1104@gmail.com
 * @Date: 2023-05-06 21:31:18
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-14 23:38:37
 * @FilePath: /espeon/code/DataStructure/src/bt/BinTree.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <unistd.h>
#include <iostream>

#include "BinTree.h"
#include "print.h"


#define RANDOM 100

template <typename T>
BinTree<T> generateBinTree() {
    BinTree<int> bintree;
    bintree.insertAsRoot(dice(NUM));
    BinNode<int>* node = bintree.root();
    for(int i = 0; i < 6; i++){
        bintree.insertAsLC(node, dice(NUM));
        bintree.insertAsRC(node, dice(NUM));
        if(dice(NUM) % 2)
            node = node->lc;
        else
            node = node->rc;
    }
    return bintree;
}

int main(){
    system("clear");
    BinTree<int> tree = generateBinTree<int>();
    print(tree);
    printf("\n\n");
    
    print("Pre-traverse:    ");
    tree.travPre(Print<int>());
    putchar('\n');
    print("In-traverse:     ");
    tree.travIn(Print<int>());
    putchar('\n');
    print("Post-traverse:   ");
    tree.travPost(Print<int>());
    putchar('\n');
    print("Level-traverse:  "); 
    tree.travLevel(Print<int>());
    putchar('\n');

    return 0;
}
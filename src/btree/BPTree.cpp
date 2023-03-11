#include "BPTree.hpp"

int main(){
    BPlusTree node;
    node.insert(5);
    node.insert(8);
    node.insert(10);
    node.insert(15);
    node.insert(16);
    node.insert(20);
    node.insert(19);
    return 0;
}
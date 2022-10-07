#include <iostream>
#include <queue>
using namespace std;
int MAX = 2;

// BP node
struct BPlusTreeNode {
    bool IS_LEAF;
    int *key, size;
    BPlusTreeNode** ptr;
    BPlusTreeNode* parent;

public:
    BPlusTreeNode():key(new int[MAX+1]),ptr(new BPlusTreeNode*[MAX+1]), parent(nullptr){}
    ~BPlusTreeNode();
};
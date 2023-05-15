#include "./BPTNode.hpp"
// BP tree
class BPlusTree {
    BPlusTreeNode* root;
    void insertInternal(int, BPlusTreeNode*, BPlusTreeNode*, BPlusTreeNode*);
    void split(int , BPlusTreeNode*, BPlusTreeNode*);
    int insertVal(int , BPlusTreeNode*);
public:
    BPlusTree():root(nullptr){}
    void insert(int x);
};

void BPlusTree::insert(int x) {
    if (root == nullptr) {
        root = new BPlusTreeNode;
        root->key[0] = x;
        root->IS_LEAF = true;
        root->size = 1;
        root->parent = NULL;
    } else {
        BPlusTreeNode* cursor = root;
        BPlusTreeNode* parent;

        while (cursor->IS_LEAF == false) {
            parent = cursor;
            for (int i = 0; i < cursor->size; i++) {
                if (x < cursor->key[i]) {
                    cursor = cursor->ptr[i];
                    break;
                }

                if (i == cursor->size - 1) {
                    cursor = cursor->ptr[i + 1];
                    break;
                }
            }
        }
        if (cursor->size < MAX) {
            insertVal(x,cursor);
            cursor->parent = parent;
            cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
            cursor->ptr[cursor->size - 1] = NULL;
        } else split(x, parent, cursor);
    }
}

int BPlusTree::insertVal(int x, BPlusTreeNode *cursor) {
    int i = 0;
    while (x > cursor->key[i] && i < cursor->size) i++;
    for (int j = cursor->size; j > i; j--) cursor->key[j] = cursor->key[j - 1];
    cursor->key[i] = x;
    cursor->size++;
    return i;
}
void BPlusTree::split(int x, BPlusTreeNode* parent, BPlusTreeNode* cursor) {
    BPlusTreeNode* LLeaf=new BPlusTreeNode;
    BPlusTreeNode* RLeaf=new BPlusTreeNode;
    insertVal(x,cursor);
    LLeaf->IS_LEAF=RLeaf->IS_LEAF=true;
    LLeaf->size=(MAX+1)/2;
    RLeaf->size=(MAX+1)-(MAX+1)/2;
    for(int i=0;i<MAX+1;i++)LLeaf->ptr[i]=cursor->ptr[i];
    LLeaf->ptr[LLeaf->size]= RLeaf;
    RLeaf->ptr[RLeaf->size]= LLeaf->ptr[MAX];
    LLeaf->ptr[MAX] = NULL;
    for (int i = 0;i < LLeaf->size; i++) {
        LLeaf->key[i]= cursor->key[i];
    }
    for (int i = 0,j=LLeaf->size;i < RLeaf->size; i++,j++) {
        RLeaf->key[i]= cursor->key[j];
    }
    if(cursor == root){
        BPlusTreeNode* newRoot = new BPlusTreeNode;
        newRoot->key[0] = RLeaf->key[0];
        newRoot->ptr[0] = LLeaf;
        newRoot->ptr[1] = RLeaf;
        newRoot->IS_LEAF = false;
        newRoot->size = 1;
        root = newRoot;
        LLeaf->parent=RLeaf->parent=newRoot;
    }
    else {insertInternal(RLeaf->key[0],parent,LLeaf,RLeaf);}

}
void BPlusTree::insertInternal(int x, BPlusTreeNode* cursor, 
                                      BPlusTreeNode* LLeaf,BPlusTreeNode* RRLeaf)
{

    if (cursor->size < MAX) {
       auto i=insertVal(x,cursor);
        for (int j = cursor->size;j > i + 1; j--) {
            cursor->ptr[j]= cursor->ptr[j - 1];
            }
        cursor->ptr[i]=LLeaf;
        cursor->ptr[i + 1] = RRLeaf;
    }

    else {

        BPlusTreeNode* newLchild = new BPlusTreeNode;
        BPlusTreeNode* newRchild = new BPlusTreeNode;
        BPlusTreeNode* virtualPtr[MAX + 2];
        for (int i = 0; i < MAX + 1; i++) {
            virtualPtr[i] = cursor->ptr[i];
        }
        int i=insertVal(x,cursor);
        for (int j = MAX + 2;j > i + 1; j--) {
            virtualPtr[j]= virtualPtr[j - 1];
        }
        virtualPtr[i]=LLeaf;
        virtualPtr[i + 1] = RRLeaf;
        newLchild->IS_LEAF=newRchild->IS_LEAF = false;
        newLchild->size= (MAX + 1) / 2;
        newRchild->size= MAX - (MAX + 1) /2;
        for (int i = 0;i < newLchild->size;i++) {

            newLchild->key[i]= cursor->key[i];
        }
        for (int i = 0, j = newLchild->size+1;i < newRchild->size;i++, j++) {

            newRchild->key[i]= cursor->key[j];
        }
        for (int i = 0;i < LLeaf->size + 1;i++) {
            newLchild->ptr[i]= virtualPtr[i];
        }
        for (int i = 0, j = LLeaf->size + 1;i < RRLeaf->size + 1;i++, j++) {
            newRchild->ptr[i]= virtualPtr[j];
        }
        if (cursor == root) {
            BPlusTreeNode* newRoot = new BPlusTreeNode;
            newRoot->key[0]= cursor->key[newLchild->size];
            newRoot->ptr[0] = newLchild;
            newRoot->ptr[1] = newRchild;
            newRoot->IS_LEAF = false;
            newRoot->size = 1;
            root = newRoot;
            newLchild->parent=newRchild->parent=newRoot;
        }
        else {
            insertInternal(cursor->key[newLchild->size],cursor->parent,newLchild,newRchild);
        }
    }
}
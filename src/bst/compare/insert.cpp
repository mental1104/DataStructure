#include "../../def.hpp"
 /*
  * para[1] = scale 
  * para[2] = method : 0-AVL/1-RedBlack/2-Splay
  * 
  */
int main(int argc, char** argv){
    if(argc!=3){
        printf("Usage: %s <num> <method>", argv[0]);
        return 0;
    }

    int scale = atoi(argv[1]);
    int method = atoi(argv[2]);

    Vector<int> vec;
    for(int i = 0; i < scale; i++)
        vec.insert(i);
    vec.unsort();
    sleep(1);

    clock_t start, end;
    AVL<int> avl;
    RedBlack<int> rb;
    Splay<int> splay;
    BTree<int> btree;

    start = clock();
    switch(method){
        case 0: 
            for(int i = 0; i < scale; i++)
                avl.insert(vec[i]);
            break;
        case 1:
            for(int i = 0; i < scale; i++)
                rb.insert(vec[i]);
            break;
        case 2:
            for(int i = 0; i < scale; i++)
                splay.insert(vec[i]);
            break;
        case 3:
            for(int i = 0; i < scale; i++)
                btree.insert(vec[i]);
            break;
        default:
            exit(-1);
    }
    end = clock();

    std::ofstream output;
    output.open("./data.txt", std::ios::app | std::ios::out);
    output << double(end-start)/CLOCKS_PER_SEC << " ";
    
    if(method == 0)
        printf("Insert: AVL size: %d\n", avl.size());
    else if(method == 1)
        printf("Insert: RBTree size: %d\n", rb.size());
    else if(method == 2)
        printf("Insert: Splay size: %d\n", splay.size());
    else if(method == 3)
        printf("Insert: BTree size: %d\n", btree.size());
    return 0;
}
#include "../../def.hpp"

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
    for(int i = 0; i < scale; i++){
        avl.insert(vec[i]);
        rb.insert(vec[i]);
        splay.insert(vec[i]);
        btree.insert(vec[i]);
    }

    start = clock();
    switch(method){
        case 0: 
            for(int i = 0; i < scale; i++)
                avl.search(dice(scale));
            break;
        case 1:
            for(int i = 0; i < scale; i++)
                rb.search(dice(scale));
            break;
        case 2:
            for(int i = 0; i < scale; i++)
                splay.search(dice(scale));
            break;
        case 3:
            for(int i = 0; i < scale; i++)
                btree.search(dice(scale));
            break;
        default:
            exit(-1);
    }
    end = clock();

    std::ofstream output;
    output.open("./data.txt", std::ios::app | std::ios::out);
    output << double(end-start)/CLOCKS_PER_SEC << " ";

    if(method == 0)
        printf("Search: AVL size: %d\n", avl.size());
    else if(method == 1)
        printf("Search: RBTree size: %d\n", rb.size());
    else if(method == 2)
        printf("Search: Splay size: %d\n", splay.size());
    else if(method == 3)
        printf("Search: BTree size: %d\n", splay.size());    
        
    return 0;
}
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

    switch(method){
        case 0: 
            for(int i = 0; i < scale; i++)
                avl.insert(vec[i]);
            start = clock();
            for(int i = 0; i < scale; i++)
                avl.search(i);
            break;
        case 1:
            for(int i = 0; i < scale; i++)
                rb.insert(vec[i]);
            start = clock();
            for(int i = 0; i < scale; i++)
                rb.search(i);
            break;
        default:
            exit(-1);
    }
    end = clock();

    std::ofstream output;
    output.open("./data.txt", std::ios::app | std::ios::out);
    output << double(end-start)/CLOCKS_PER_SEC << " ";
    return 0;
}
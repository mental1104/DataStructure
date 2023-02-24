#include "../def.hpp"

int main(){

    AVL<int> bst;
    system("clear");

    for(int i = 0; i < 16; i+=2){
        bst.insert(i);
    }
    print(bst);
    bst.remove(8);
    printf("========================\n");
    print(bst);
    printf("========================\n");
    bst.insert(8);
    print(bst);
    return 0;
}
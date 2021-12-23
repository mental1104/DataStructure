#include "../def.hpp"

int main(){

    AVL<int> bst;
    system("clear");

    //插入50个[0,100)的整数
    for(int i = 0; i < 1000000; i++){
        bst.insert(i);
    }
    print(bst);
    
    return 0;
}
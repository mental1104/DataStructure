#include "BST.h"

int main(){
    BST<int> bst;
    int temp;

    for(int i = 0; i < 10; i++){
        bst.insert(i);
    }

    //print(bst);
    
    /*
    printf("Press to see if you want to see the process of deletion\n");
    getchar();
    system("clear");

    while(!bst.empty()){
        temp = dice(RANDOM);
        if(bst.search(temp)){
            printf("Delete: %d\n\n", temp);
            bst.remove(temp);
            print(bst);
            sleep(1);
            system("clear");
        }
    }
    */
    return 0;
}
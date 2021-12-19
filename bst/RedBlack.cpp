#include "../def.hpp"

int main(){
    system("clear");
    RedBlack<int> tree;
    int temp;

    for(int i = 0; i < 100000; i++){
        temp = dice(10000);
        tree.insert(temp);
        //sleep(1);
        //system("clear");
    }
    print(tree);
    sleep(100);

    /*
    printf("Press to see if you want to see the process of deletion\n");
    getchar();
    system("clear");

    
    while(!tree.empty()){
        temp = dice(RANDOM);
        if(tree.search(temp)){
            printf("Delete: %d\n\n", temp);
            tree.remove(temp);
            printRedBlackTree(tree);
            sleep(1);
            system("clear");
        }
    }
    */
    return 0;
}
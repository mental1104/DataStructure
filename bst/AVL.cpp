#include "../def.hpp"

#define N 40
#define RANDOM 100

int main(){

    AVL<int> bst;
    int temp;
    system("clear");

    for(int i = 0; i < N; i++){
        temp = dice(RANDOM);
        printf("Insert: %d\n\n", temp);
        bst.insert(temp);
        print(bst);
        sleep(1);
        system("clear");
    }

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

    return 0;
}
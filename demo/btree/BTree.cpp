#include "BTree.h"
#include "print.h"

#define RANDOM 100
#define N 100

int main(int argc, char** argv){

    BTree<int> btree{8};
    int temp;

    system("clear");
    for(int i = 1; i < N; i++){
        temp = dice(RANDOM);
        printf("Insert: %d\n\n", temp);

        btree.insert(temp);
        print(btree);
        sleep_seconds(1); // sleep 在 Windows 缺失，改用跨平台封装
        system("clear");
    }

    printf("Press to continue..(Deletion)\n");
    getchar();
    system("clear");

    while(!btree.empty()){
        temp = dice(RANDOM);
        if(btree.search(temp)){
            printf("Delete: %d\n\n", temp);
            btree.remove(temp);
            print(btree);
            sleep_seconds(1); // sleep 在 Windows 缺失，改用跨平台封装
            system("clear");
        }
    }
    
    return 0;
}

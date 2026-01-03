#include "BTree.h"
#include "print.h"

#define RANDOM 100
#define N 100

int main(int argc, char** argv){
    (void)argc;
    (void)argv;

    BTree<int> btree{8};
    int temp;

    clear_screen();
    for(int i = 1; i < N; i++){
        temp = dice(RANDOM);
        printf("Insert: %d\n\n", temp);

        btree.insert(temp);
        print(btree);
        sleep_seconds(1); // sleep 在 Windows 缺失，改用跨平台封装
        clear_screen();
    }

    printf("Press to continue..(Deletion)\n");
    getchar();
    clear_screen();

    while(!btree.empty()){
        temp = dice(RANDOM);
        if(btree.search(temp)){
            printf("Delete: %d\n\n", temp);
            btree.remove(temp);
            print(btree);
            sleep_seconds(1); // sleep 在 Windows 缺失，改用跨平台封装
            clear_screen();
        }
    }
    
    return 0;
}

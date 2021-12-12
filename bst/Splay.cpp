#include "../def.hpp"
#define RANDOM 100
#define N 40

int main(int argc, char** argv){

    Splay<int> S;
    int temp;
    system("clear");

    //insert
    for(int i = 0; i < N; i++){
        temp = dice(RANDOM);
        printf("Insert: %d\n\n", temp);
        S.insert(temp);
        print(S);
        sleep(1);
        system("clear");
    }
   
    //search
    printf("Press to continue..(Search)\n");
    getchar();

    for(int i = 0; i < N/10;){
        temp = dice(RANDOM);
        if(S.search(temp)->data == temp){
            i++;
            printf("Search: %d\n\n", temp);
            print(S);
            sleep(4);
            system("clear");
        }
    }

    //delete
    printf("Press to continue..(Deletion)\n");
    getchar();
    
    while(!S.empty()){
        temp = dice(RANDOM);
        if(S.search(temp)->data == temp){
            printf("Delete: %d\n\n", temp);
            S.remove(temp);
            print(S);
            sleep(1);
            system("clear");
        }
    }


    return 0;
}

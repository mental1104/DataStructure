#include <iostream>

#include "Skiplist.h"
#include "print.h"

int main(){
    system("clear");
    Skiplist<int, int> list;
    for(int i = 0; i < 8; i+=2){
        list.put(i, dice(10000));
        print(list);
        sleep(1);
        system("clear");
    }

    for(int i = 1; i < 7; i+=2){
        list.put(i, dice(10000));
        print(list);
        sleep(1);
        system("clear");
    }

    list.put(7, dice(10000));
    print(list);
    getchar();
    system("clear");

    std::cout << list.size() << std::endl;
    return 0;
}
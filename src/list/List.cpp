#include "List.h"
#include <iostream>

int main(){
    List<int> list;
    int temp;

    for(int i = 0; i < 10; i++){
        temp = dice(1000);
        list.insertAsLast(temp);
    }
    std::cout << list.size() << std::endl;
    
    //print(list);
    putchar('\n');
    list.sort();
    //print(list);
    return 0;
} 

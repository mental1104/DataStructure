#include <iostream>

#include "List.h"
#include "print.h"

int main(){
    List<int> list;
    int temp;

    for(int i = 0; i < 10; i++){
        temp = dice(1000);
        list.insertAsLast(temp);
    }
    std::cout << list.size() << std::endl;
    
    print(list);
    putchar('\n');
    list.sort();
    print(list);

    for(auto i : list)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
} 

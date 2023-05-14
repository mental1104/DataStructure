#include <iostream>

#include "List.h"
#include "print.h"
#include "rand.h"

static int times = 30;

int main(){
    int random =  100;
    List<int> list;
    display(list);

    std::cout << "Final traverse: " << std::endl;

    for(auto i : list)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
} 

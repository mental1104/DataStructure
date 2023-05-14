#include <iostream>

#include "Vector.h"
#include "rand.h"
#include "print.h" 

static int times = 30;


int main(){
    int random =  100;
    Vector<int> vec;
    display(vec);

    std::cout << "Final traverse: " << std::endl;

    for(auto i : vec)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}
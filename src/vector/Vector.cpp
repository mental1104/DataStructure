#include <iostream>

#include "Vector.h"
#include "print.h" 

int main(){
    Vector<int> vec;
    for(int i = 0; i < 100; i++)
        vec.insert(i);
    print(vec);
    for(auto i : vec)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}
#include "Vector.h"
#include <iostream>

int main(){
    Vector<int> vec;
    for(int i = 0; i < 10; i++)
        vec.insert(i);
    std::cout << vec.size() << std::endl;
    return 0;
}
#include <iostream>

#include "Vector.h"
#include "print.h" 


int main(){
    Vector<int> vec;
    for(int i = 0; i < 10; i++)
        vec.insert(i);
    print(vec);
    return 0;
}
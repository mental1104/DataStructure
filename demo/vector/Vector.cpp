/*
 * @Date: 2023-05-06 19:43:26
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-15 23:07:04
 */
#include <iostream>

#include "Vector.h"
#include "Sort.h"
#include "rand.h"
#include "print.h" 

int main(){
    Vector<int> vec;
    display(vec);

    std::cout << "Final traverse: " << std::endl;
    Sort(vec);
    for(auto i : vec)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}

/*
 * @Author: mental1104 mental1104@gmail.com
 * @Date: 2023-05-06 19:43:26
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-15 00:30:43
 * @FilePath: /espeon/code/DataStructure/src/bst/AVL.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>

#include "AVL.h"
#include "print.h"

int main(){
    AVL<int> avl;
    display(avl);

    std::cout << "Final traverse: " << std::endl;

    for(auto i : avl)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}

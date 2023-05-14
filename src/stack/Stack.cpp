/*
 * @Author: mental1104 mental1104@gmail.com
 * @Date: 2023-05-06 19:43:26
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-14 22:29:09
 * @FilePath: /espeon/code/DataStructure/src/stack/Stack.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>

#include "Stack.h"
#include "print.h"

int main(){
    Stack<int> S;
    S.push(1);
    S.push(2);
    print(S);
    S.pop();
    print(S);
    std::cout << S.size() << std::endl;
    for(int i = 0; i < 10; i++)
        S.push(i);
    for(auto i : S)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}
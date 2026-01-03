/*
 * @Author: mental1104 mental1104@gmail.com
 * @Date: 2023-05-06 19:43:26
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-14 23:49:25
 * @FilePath: /espeon/code/DataStructure/src/stack/Stack.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>

#include "Stack.h"
#include "rand.h"
#include "print.h"

const int times = 10;

int main(){
    Stack<int> S;
    for(int i = 1; i < times; i++){
        int num;
        clear_screen();
        if(i % 3 == 0){
            num = S.pop();
            std::cout << "Pop: \t" << num << std::endl;   
        }
        else {
            num = dice(100);
            S.push(num);
            std::cout << "Push: \t" << num << std::endl;  
        }
            
        print(S);
        sleep_seconds(1); // Windows 无 sleep，使用跨平台封装
    }

    std::cout << "Final traverse" << std::endl;
    for(auto i : S)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}

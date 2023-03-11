#include "Stack.h"
#include <iostream>

int main(){
    Stack<int> S;
    S.push(1);
    S.push(2);
    //print(S);
    S.pop();
    //print(S);
    std::cout << S.size() << std::endl;
    return 0;
}
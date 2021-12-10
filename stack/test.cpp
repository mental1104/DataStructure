#include "./Stack.hpp"

int main(){
    Stack<int> S;
    S.push(1);
    S.push(2);
    print(S);
    S.pop();
    print(S);
    return 0;
}
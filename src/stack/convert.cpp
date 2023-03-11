#include "Stack.h"
#include "print.h"

void convertA(Stack<char>& S, int n, int base){
    static char digit[]
    = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    if(0 < n){
        S.push(digit[n % base]);
        convertA(S, n/base, base);
    }
    return;
}

void convertB(Stack<char>& S, int n, int base){
    static char digit[]
    = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    while(n > 0){
        int remainder = (int)(n % base);
        S.push(digit[remainder]);
        n /= base;
    }
    return;
}

int main(){
    //十进制转十六进制：
    Stack<char> hex;
    convertA(hex, 65535, 16);
    print(hex);
    //十进制转八进制：
    Stack<char> oct;
    convertB(oct, 32, 8);
    print(oct);

    return 0;
}
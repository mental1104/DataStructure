#include "../../def.hpp"

bool paren(const char exp[], int lo, int hi){
    Stack<char> S;
    for(int i = lo; i <= hi; i++){
        switch(exp[i]){
            case '(': 
            case '[':
            case '{':
                S.push(exp[i]);
                break;
            case ')':
                if((S.empty()) || ('(' != S.pop())) 
                    return false;
                break;
            case ']':
                if((S.empty()) || ('[' != S.pop()))
                    return false;
                break;
            case '}':
                if((S.empty()) || ('{' != S.pop()))
                    return false;
                break;
            default:
                break;
        }
    }
    return S.empty();
}

int main(){
    printf("%d\n", paren("{()[({})]}", 0, 10));
    return 0;
}
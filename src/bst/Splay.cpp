#include <iostream>

#include "Splay.h"
#include "print.h"

#define RANDOM 100
#define N 40

int main(int argc, char** argv){

    Splay<int> S;
    Vector<int> vec;
    //插入50个[0,100)的整数
    for(int i = 0; i < 1000000; i++){
        vec.insert(i);
    }
    vec.unsort();
    for(int i = 0; i < 1000000; i++){
        S.insert(vec[i]);
    }
    for(int i = 0; i < 1000000; i++){
        S.search(i);
    }
    printf("%d\n", S.size());
    return 0;
}

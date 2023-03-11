#include <iostream>

#include "Heap.h"
#include "print.h"

int main(){
    Heap<int> in;
    in.insert(4);
    in.insert(3);
    in.insert(0);
    in.insert(1);
    in.insert(2);
    print(in);
    in.insert(5);
    print(in);
    Heap<int> re;
    re.insert(5);
    re.insert(4);
    re.insert(3);
    re.insert(2);
    re.insert(0);
    re.insert(1);
    //print(re);
    re.delMax();
    //print(re);

    std::cout << re.size() << std::endl;
    return 0;
}
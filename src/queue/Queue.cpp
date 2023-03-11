#include "Queue.h"
#include <iostream>

int main(){
    Queue<int> Q;
    Q.enqueue(1);
    Q.enqueue(2);
    //print(S);
    Q.dequeue();
    //print(S);
    std::cout << Q.size() << std::endl;
    return 0;
}
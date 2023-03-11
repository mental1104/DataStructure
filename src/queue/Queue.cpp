#include <iostream>

#include "Queue.h"
#include "print.h"

int main(){
    Queue<int> Q;
    Q.enqueue(1);
    Q.enqueue(2);
    print(Q);
    Q.dequeue();
    print(Q);
    std::cout << Q.size() << std::endl;
    return 0;
}
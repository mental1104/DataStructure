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
    for(int i = 0; i < 10; i++)
        Q.enqueue(i);
    for(auto i : Q)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}
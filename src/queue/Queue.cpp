#include <iostream>

#include "Queue.h"
#include "rand.h"
#include "print.h"

const int times = 10;

int main(){
    Queue<int> Q;
    for(int i = 1; i < times; i++){
        int num;
        system("clear");
        if(i % 3 == 0){
            num = Q.dequeue();
            std::cout << "Dequeue: \t" << num << std::endl;   
        }
        else {
            num = dice(100);
            Q.enqueue(num);
            std::cout << "Enqueue: \t" << num << std::endl;  
        }
            
        print(Q);
        sleep(1);
    }
    
    std::cout << "Final traverse" << std::endl;
    for(auto i : Q)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}
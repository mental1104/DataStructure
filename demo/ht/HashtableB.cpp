#include <iostream>

#include "HashtableB.h"
#include "print.h"

int main(){
    system("clear");
    QuadraticHT<int, int> ht;
    for(int i = 0; i < 100000; i++)
        ht.put(i, i);
    std::cout << ht.size() << std::endl;
    print(ht);

    for(int i = 0, elem = 0; i < 7;i++, elem+=7){
        system("clear");
        ht.put(elem, elem);
        print(ht);
        sleep(1);
    } //Conflict on purpose by the multiples of 7

    return 0;
}
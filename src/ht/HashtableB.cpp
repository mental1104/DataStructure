#include "HashtableB.h"
#include <iostream>

int main(){
    //system("clear");
    QuadraticHT<int, int> ht;
    for(int i = 0; i < 100000; i++)
        ht.put(i, i);
    std::cout << ht.size() << std::endl;
    //print(ht);
    /*
    for(int i = 0, elem = 0; i < 7;i++, elem+=7){
        system("clear");
        ht.put(elem, elem);
        print(ht);
        sleep(1);
    }//Conflict on purpose by the multiples of 7

    for(int i = 0; i < 1000; i++){
        system("clear");
        ht.put(dice(1000), dice(1000));
        print(ht);
        sleep(1);
    }*/
    /*
    for(int i = 0; i < 100; i++){
        system("clear");
        ht.put(elem, elem);
        print(ht);
        sleep(1);
    }*/
    //print(ht);
    //getchar();
    //system("clear");
    return 0;
}
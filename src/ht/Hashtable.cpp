#include <iostream>

#include "Hashtable.h"
#include "print.h"


int main(){
    system("clear");
    Hashtable<int, int> ht;
    for(int i = 1; i < 10000; i++){
        ht.put(i,dice(i));
        print(ht);
        sleep(1);
        system("clear");
    }
    ht.put(dice(100), dice(100));
    std::cout << ht.size() << std::endl;

    system("clear");
    return 0;
}
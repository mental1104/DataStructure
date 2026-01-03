#include <iostream>

#include "HashtableB.h"
#include "print.h"

int main(){
    clear_screen();
    QuadraticHT<int, int> ht;
    for(int i = 0; i < 100000; i++)
        ht.put(i, i);
    std::cout << ht.size() << std::endl;
    print(ht);

    for(int i = 0, elem = 0; i < 7;i++, elem+=7){
        clear_screen();
        ht.put(elem, elem);
        print(ht);
        sleep_seconds(1); // Windows 无 sleep，使用跨平台封装
    } //Conflict on purpose by the multiples of 7

    return 0;
}

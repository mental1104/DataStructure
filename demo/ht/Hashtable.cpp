#include <iostream>

#include "Hashtable.h"
#include "print.h"


int main(){
    clear_screen();
    Hashtable<int, int> ht;
    for(int i = 1; i < 10000; i++){
        ht.put(i,dice(i));
        print(ht);
        sleep_seconds(1); // Windows 无 sleep，使用跨平台封装
        clear_screen();
    }
    ht.put(dice(100), dice(100));
    std::cout << ht.size() << std::endl;

    clear_screen();
    return 0;
}

#include <iostream>

#include "Skiplist.h"
#include "print.h"

int main(){
    clear_screen();
    Skiplist<int, int> list;
    for(int i = 0; i < 8; i+=2){
        list.put(i, dice(10000));
        print(list);
        sleep_seconds(1); // Windows 无 sleep，使用跨平台封装
        clear_screen();
    }

    for(int i = 1; i < 7; i+=2){
        list.put(i, dice(10000));
        print(list);
        sleep_seconds(1); // Windows 无 sleep，使用跨平台封装
        clear_screen();
    }

    list.put(7, dice(10000));
    print(list);
    getchar();
    clear_screen();

    std::cout << list.size() << std::endl;
    return 0;
}

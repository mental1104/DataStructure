#include "../def.hpp"  
#include <unistd.h>

int main(){
    List<int> list;
    int temp;

    for(int i = 0; i < 10; i++){
        temp = dice(1000);
        list.insertAsLast(temp);
    }
    print(list);
    putchar('\n');
    list.sort();
    print(list);
    return 0;
} 

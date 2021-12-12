#include "../def.hpp"

int main(){
    system("clear");
    Hashtable<int, int> ht;
    for(int i = 1; i < 10000; i++){
        ht.put(dice(i),dice(i));
        print(ht);
        sleep(1);
        system("clear");
    }
    ht.put(dice(100), dice(100));
    print(ht);
    getchar();
    system("clear");
    return 0;
}
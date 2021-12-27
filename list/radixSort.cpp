#include "../def.hpp"

int main(){
    List<U> list;
    for(U i = 0; i < 100; i++){
        list.insertAsLast(dice(100));
    }
    print(list);
    printf("After radixSorting...\n");
    list.radixSort(list.first(), list.size());
    print(list);
    return 0;
}
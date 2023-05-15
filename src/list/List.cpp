/*
 * @Date: 2023-05-06 19:43:26
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-15 23:44:13
 */
#include <iostream>

#include "List.h"
#include "Sort.h"
#include "print.h"
#include "rand.h"

static int times = 30;

int main(){
    {
        int random =  100;
        List<int> list;
        display(list);

        std::cout << "Final traverse: " << std::endl;
        Sort(list, SortStrategy::InsertionSort);
        for(auto i : list)
            std::cout << i << " ";
        std::cout << std::endl;
    }

    {
        // radixSort
        std::cout << "--------------" << std::endl;
        List<U> list;
        for(U i = 0; i < 100; i++){
            list.insertAsLast(dice(100));
        }
        print(list);
        printf("After radixSorting...\n");
        Sort(list, SortStrategy::RadixSort);
        print(list);
    }
    
    return 0;
} 

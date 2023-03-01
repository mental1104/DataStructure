#include "rand.h"
#include "Vector.h"
#include <iostream>
int main(){
    /*
    Vector<int> vec;
    for(int i = 0; i < 11; i++){
        if(i%2 == 0)
            vec.insert(23);
        else 
            vec.insert(i);
    }
    print(vec);
    vec.unsort();
    print(vec);
    print(vec.majEleCandidate());
    printf("\n");
    vec.range(2);
    printf("\n");*/
    Vector<int> dup;
    for(int i = 0; i < 20; i++){
        dup.insert(dice(5));
    }
    for(int i = 0; i < 20; i++){
        std::cout << dup[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}
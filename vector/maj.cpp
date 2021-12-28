#include "../def.hpp"

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

    print(dup);
    for(int i = 2; i < 10; i++){
        printf("occurrence beyond 1/%d: ", i);
        dup.range(i);
    }

    return 0;
}
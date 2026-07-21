#ifndef __DSA_ERATOSTHENES
#define __DSA_ERATOSTHENES

#include "Bitmap.h"

Bitmap* eratosthenes(int n){
    Bitmap* B = new Bitmap(n);
    B->set(0);
    B->set(1);
    for(int i = 2; i < n; i++)
        if(!B->test(i))
            for(int j = 2*i; j < n; j += i)
                B->set(j);
    return B;
}

void eratosthenes_to_file(int n, const char* file){
    Bitmap* B = eratosthenes(n);
    B->dump(file);
    delete B;
    B = nullptr;
    return;
}

#endif
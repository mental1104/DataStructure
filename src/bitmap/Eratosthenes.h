#pragma once
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

void eratosthenes_to_file(int n, char* file){
    Bitmap B(n);
    B.set(0);
    B.set(1);
    for(int i = 2; i < n; i++)
        if(!B.test(i))
            for(int j = 2*i; j < n; j += i)
                B.set(j);
    B.dump(file);
    return;
}
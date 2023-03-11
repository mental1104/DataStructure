#include "Bitmap.h"

void Eratosthenes(int n, char* file){
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

int main(){
    Eratosthenes(10, "./100-Prime.txt");
    return 0;
}
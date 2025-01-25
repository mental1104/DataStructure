#ifndef __DSA_LSD
#define __DSA_LSD

#include "Vector.h"
#include "dsa_string.h"  

void LSD(Vector<String>& a, int W){
    int N = a.size();
    int R = 128;
    Vector<String> aux(N, N, String());

    for(int d = W-1; d >= 0; d--){
        Vector<int> count(R+1, R+1, 0);

        for(int i = 0; i < N; i++)// Compute frequency counts
            count[a[i][d]+1]++;
        for(int r = 0; r < R; r++)// Transform counts to indices
            count[r+1] += count[r];
        for(int i = 0; i < N; i++)// Distribute
            aux[count[a[i][d]]++] = a[i];
        for(int i = 0; i < N; i++)// Copy back
            a[i] = aux[i];
    }
    return;
}

#endif
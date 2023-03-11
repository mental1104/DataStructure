#pragma once 
#include "String.h"
#include "Vector.h"

#define R 128


class MSD {
private:
    static char charAt(const String& s, int d);
    static void sort(Vector<String>& a, Vector<String>& aux, int lo, int hi, int d);
    static void insertionSort(Vector<String>& a, int lo, int hi, int d);
public:
    static void sort(Vector<String>& a);  
};

char MSD::charAt(const String& s, int d){
    if(d < s.size())
        return s[d];
    else 
        return -1;
}

void MSD::sort(Vector<String>& a){
    int N = a.size();
    Vector<String> aux(N, N, String());
    sort(a, aux, 0, N-1, 0);
}

void MSD::sort(Vector<String>& a, Vector<String>& aux, int lo, int hi, int d){
    if(hi <= lo + 3){
        insertionSort(a, lo, hi, d);
        return;
    }

    Vector<int> count(R+2, R+2, 0);         //Compute frequency counts
    for(int i = lo; i <= hi; i++)
        count[charAt(a[i], d)+2]++;

    for(int r = 0; r < R+1; r++)    //Transform counts to indices
        count[r+1] += count[r];
    
    for(int i = lo; i <= hi; i++)   //Distribute
        aux[count[charAt(a[i], d)+1]++] = a[i];
    
    for(int i = lo; i <= hi; i++)   //copy back
        a[i] = aux[i-lo];
    
    for(int r = 0; r < R; r++)
        sort(a, aux, lo + count[r], lo + count[r+1]-1, d+1);
}

void MSD::insertionSort(Vector<String>& a, int lo, int hi, int d){
    for(int i = lo; i <= hi; i++)
        for(int j = i; j > lo && (a[j].substr(d) < a[j-1].substr(d)); --j)
            swap(a[j], a[j-1]);
}
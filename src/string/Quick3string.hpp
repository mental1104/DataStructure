#pragma once
#include "../def.hpp"

class Quick3string{
private:
    static int charAt(const String& s, int d);
    static void sort(Vector<String>& a, int lo, int hi, int d);
public:
    static void sort(Vector<String>& a);
};

int Quick3string::charAt(const String& s, int d){
    if(d < s.size())
        return s[d];
    else 
        return -1;
}

void Quick3string::sort(Vector<String>& a){
    sort(a, 0, a.size()-1, 0);
}

void Quick3string::sort(Vector<String>& a, int lo, int hi, int d){
    if(hi <= lo) return;

    int lt = lo, gt = hi;
    int v = charAt(a[lo], d);
    int i = lo + 1;
    while(i <= gt){
        int t = charAt(a[i], d);
        if      (t < v) swap(a[lt++], a[i++]);
        else if (t > v) swap(a[i], a[gt--]);
        else            i++;
    }

    sort(a, lo, lt-1, d);
    if (v >= 0) sort(a, lt, gt, d+1);
    sort(a, gt+1, hi, d);
}
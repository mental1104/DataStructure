#ifndef __DSA_MSD
#define __DSA_MSD

#include "dsa_string.h"
#include "Vector.h"

class MSD {
private:
    static const int kRadix = 128;

    static int charAt(const String& s, int d);
    static void sort(Vector<String>& a, Vector<String>& aux, int lo, int hi, int d);
    static void insertionSort(Vector<String>& a, int lo, int hi, int d);

public:
    static void sort(Vector<String>& a);
};

int MSD::charAt(const String& s, int d) {
    if (d >= 0 && static_cast<size_type>(d) < s.size())
        return s[static_cast<size_type>(d)];
    return -1;
}

void MSD::sort(Vector<String>& a) {
    int count = a.size();
    Vector<String> aux(count, count, String());
    sort(a, aux, 0, count - 1, 0);
}

void MSD::sort(Vector<String>& a, Vector<String>& aux, int lo, int hi, int d) {
    if (hi <= lo + 3) {
        insertionSort(a, lo, hi, d);
        return;
    }

    Vector<int> count(kRadix + 2, kRadix + 2, 0);
    for (int index = lo; index <= hi; ++index)
        ++count[charAt(a[index], d) + 2];

    for (int radix = 0; radix < kRadix + 1; ++radix)
        count[radix + 1] += count[radix];

    for (int index = lo; index <= hi; ++index)
        aux[count[charAt(a[index], d) + 1]++] = a[index];

    for (int index = lo; index <= hi; ++index)
        a[index] = aux[index - lo];

    for (int radix = 0; radix < kRadix; ++radix)
        sort(a, aux, lo + count[radix], lo + count[radix + 1] - 1, d + 1);
}

void MSD::insertionSort(Vector<String>& a, int lo, int hi, int d) {
    for (int index = lo; index <= hi; ++index) {
        for (int current = index;
             current > lo && a[current].substr(d) < a[current - 1].substr(d);
             --current) {
            swap(a[current], a[current - 1]);
        }
    }
}

#endif


#pragma once

#include <cstdio>
#include <cstring>
#include "MatchObserver.h"
#include "dsa_string.h"

enum class KMPStrategy { Basic, Improved };

inline int* buildNext(const String& P) {
    const char* pc = P.c_str();
    int m = (int)P.size(), j = 0;
    int* next = new int[m]; int t = next[0] = -1;
    while (j < m - 1)
        if (0 > t || pc[t] == pc[j]) {
            ++t; ++j; next[j] = t;
        } else
            t = next[t];
    return next;
}

inline int* buildNextImproved(const String& P) {
    const char* pc = P.c_str();
    int m = (int)P.size(), j = 0;
    int* next = new int[m]; int t = next[0] = -1;
    while (j < m - 1)
        if (0 <= t && pc[t] != pc[j])
            t = next[t];
        else
            if (pc[++t] != pc[++j])
                next[j] = t;
            else
                next[j] = next[t];
    return next;
}

inline int matchKMP(const String& P, const String& T, KMPStrategy strategy = KMPStrategy::Basic, MatchObserver* obs = nullptr) {
    NoopMatchObserver noop;
    MatchObserver& observer = obs ? *obs : noop;
    int* next = (strategy == KMPStrategy::Improved) ? buildNextImproved(P) : buildNext(P);
    int n = (int)T.size(), i = 0;
    int m = (int)P.size(), j = 0;
    observer.onNextTable(P, next, m);
    while ((j < m) && (i < n)) {
        observer.onProgress(T, P, i - j, j, next, m);
        observer.onPause();
        if (0 > j || T[i] == P[j]) { i++; j++; }
        else j = next[j];
    }
    delete[] next;
    return i - j;
}

inline int matchKMPBasic(const String& P, const String& T) {
    StdoutMatchObserver obs;
    return matchKMP(P, T, KMPStrategy::Basic, &obs);
}

inline int matchKMPImproved(const String& P, const String& T) {
    StdoutMatchObserver obs;
    return matchKMP(P, T, KMPStrategy::Improved, &obs);
}

#pragma once

#include <algorithm>
#include <cstdio>
#include <cstring>
#include "MatchObserver.h"
#include "dsa_string.h"

enum class BMStrategy { BadCharacter, Full };

inline int* buildBC(const String& P, MatchObserver* obs = nullptr) { // Bad Character table
    NoopMatchObserver noop;
    MatchObserver& observer = obs ? *obs : noop;
    int* bc = new int[256];
    std::fill(bc, bc + 256, -1);
    int m = (int)P.size();
    const char* pc = P.c_str();
    for (int j = 0; j < m; j++) bc[(unsigned char)pc[j]] = j;
    observer.onBCTable(bc, 256);
    return bc;
}

inline int* buildSS(const String& P, int m) { // suffix size table
    const char* pc = P.c_str();
    int* ss = new int[m];
    ss[m - 1] = m;
    for (int lo = m - 1, hi = m - 1, j = lo - 1; j >= 0; j--) {
        if ((lo < j) && (ss[m - hi + j - 1] < j - lo))
            ss[j] = ss[m - hi + j - 1];
        else {
            hi = j; lo = std::min(lo, hi);
            while ((0 <= lo) && (pc[lo] == pc[m - hi + lo - 1])) lo--;
            ss[j] = hi - lo;
        }
    }
    return ss;
}

inline int* buildGS(const String& P, int m, MatchObserver* obs = nullptr) { // Good Suffix table
    NoopMatchObserver noop;
    MatchObserver& observer = obs ? *obs : noop;
    int* ss = buildSS(P, m);
    int* gs = new int[m];
    for (int j = 0; j < m; j++) gs[j] = m;
    for (int i = 0, j = m - 1; j >= 0; j--)
        if (j + 1 == ss[j])
            while (i < m - j - 1) gs[i++] = m - j - 1;
    for (int j = 0; j < m - 1; j++) gs[m - ss[j] - 1] = m - j - 1;
    observer.onGSTable(gs, m, P);
    delete[] ss;
    return gs;
}

inline int matchBM(const String& P, const String& T, BMStrategy strategy = BMStrategy::BadCharacter, MatchObserver* obs = nullptr) {
    NoopMatchObserver noop;
    MatchObserver& observer = obs ? *obs : noop;
    int m = (int)P.size();
    int n = (int)T.size();
    if (m <= 0 || n <= 0) return 0;

    int* bc = buildBC(P, &observer);
    if (strategy == BMStrategy::BadCharacter) {
        int i = 0, j = 0;
        for (i = 0; i + m <= n; i += std::max(1, j - bc[(unsigned char)T[i + j]])) {
            for (j = m - 1; (0 <= j) && (P[j] == T[i + j]); j--);
            observer.onProgress(T, P, i, j, bc, 256);
            observer.onPause();
            if (j < 0) break;
        }
        delete[] bc;
        return i;
    }

    int* gs = buildGS(P, m, &observer);
    size_t i = 0;
    while (i + (size_t)m <= (size_t)n) {
        int j = m - 1;
        while (P[j] == T[i + j]) {
            observer.onProgress(T, P, (int)i, j, gs, m);
            observer.onPause();
            if (0 > --j) break;
        }
        if (0 > j) {
            observer.onProgress(T, P, (int)i, j, gs, m);
            observer.onPause();
            break;
        }
        observer.onProgress(T, P, (int)i, j, gs, m);
        observer.onPause();
        i += std::max(gs[j], j - bc[(unsigned char)T[i + j]]);
    }
    delete[] gs;
    delete[] bc;
    return (int)i;
}

inline int matchBMBadCharacter(const String& P, const String& T) {
    return matchBM(P, T, BMStrategy::BadCharacter, nullptr);
}

inline int matchBMFull(const String& P, const String& T) {
    return matchBM(P, T, BMStrategy::Full, nullptr);
}

inline int matchBMVerbose(const String& P, const String& T, BMStrategy strategy = BMStrategy::BadCharacter) {
    StdoutMatchObserver obs;
    return matchBM(P, T, strategy, &obs);
}

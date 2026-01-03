#pragma once

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "MatchObserver.h"
#include "dsa_string.h"

constexpr int kKRMod = 97;   // a prime to reduce collisions
constexpr int kKRBase = 10;  // radix for decimal strings

inline int krDigit(const String& S, size_type i) { return S[i] - '0'; }

using HashCode = std::int64_t;

inline HashCode prepareDm(size_type m) { // R^(m - 1) % M
    HashCode Dm = 1;
    for (size_type i = 1; i < m; i++) Dm = (kKRBase * Dm) % kKRMod;
    return Dm;
}

inline bool check1by1(const String& P, const String& T, size_type i) { // confirm exact match
    for (size_type m = P.size(), j = 0; j < m; j++, i++)
        if (P[j] != T[i]) return false;
    return true;
}

inline void updateHash(HashCode& hashT, const String& T, size_type m, size_type k, HashCode Dm) {
    hashT = (hashT - krDigit(T, k - 1) * Dm) % kKRMod;
    hashT = (hashT * kKRBase + krDigit(T, k + m - 1)) % kKRMod;
    if (hashT < 0) hashT += kKRMod;
}

inline int matchKR(const String& P, const String& T, MatchObserver* obs = nullptr) { // Karp-Rabin
    NoopMatchObserver noop;
    MatchObserver& observer = obs ? *obs : noop;
    size_type m = P.size(), n = T.size();
    if (m == 0 || n == 0 || m > n) return 0;

    HashCode Dm = prepareDm(m), hashP = 0, hashT = 0;
    for (size_type i = 0; i < m; i++) {
        hashP = (hashP * kKRBase + krDigit(P, i)) % kKRMod;
        hashT = (hashT * kKRBase + krDigit(T, i)) % kKRMod;
    }

    for (size_type k = 0;;) {
        observer.onKRProgress(T, P, static_cast<size_t>(k), hashP, hashT);
        observer.onPause();
        if (hashT == hashP && check1by1(P, T, k)) return static_cast<int>(k);
        if (++k > n - m) return static_cast<int>(k);
        updateHash(hashT, T, m, k, Dm);
    }
}

inline int matchKR(const String& P, const String& T) {
    StdoutMatchObserver obs;
    return matchKR(P, T, &obs);
}

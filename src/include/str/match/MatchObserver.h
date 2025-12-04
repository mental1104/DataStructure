#pragma once

#include <cstdio>
#include <cstring>
#include "dsa_string.h"

inline void moPrintString(const String& s) {
    for (size_t m = s.size(), k = 0; k < m; k++) printf("%4c", s[k]);
}

inline void moPrintIndexLine(size_t n) {
    for (size_t t = 0; t < n; t++) printf("%4d", (int)t);
    printf("\n");
}

struct MatchObserver {
    virtual ~MatchObserver() = default;
    virtual void onNextTable(const String& /*pattern*/, const int* /*next*/, int /*len*/) {}
    virtual void onProgress(const String& /*text*/, const String& /*pattern*/, int /*align*/, int /*j*/, const int* /*aux*/, int /*auxLen*/) {}
    virtual void onBCTable(const int* /*bc*/, int /*len*/) {}
    virtual void onGSTable(const int* /*gs*/, int /*len*/, const String& /*pattern*/) {}
    virtual void onKRProgress(const String& /*text*/, const String& /*pattern*/, size_t /*k*/, long long /*hashP*/, long long /*hashT*/) {}
    virtual void onPause() {}
};

struct StdoutMatchObserver : MatchObserver {
    int step = 0;

    void onNextTable(const String& pattern, const int* next, int len) override {
        if (!next) return;
        moPrintString(pattern); printf("\n");
        moPrintIndexLine(len);
        for (int i = 0; i < len; i++) printf("%4d", next[i]);
        printf("\n\n");
    }

    void onProgress(const String& text, const String& pattern, int align, int j, const int*, int) override {
        printf("\n-- Step %2d: --\n", ++step);
        size_t n = text.size();
        moPrintIndexLine(n);
        moPrintString(text); printf("\n");
        if (0 <= align + j) { for (int t = 0; t < align + j; t++) printf("%4c", ' '); printf("%4c", '|'); }
        printf("\n");
        for (int t = 0; t < align; t++) printf("%4c", ' ');
        moPrintString(pattern); printf("\n");
    }

    void onBCTable(const int* bc, int len) override {
        printf("\n-- bc[] Table ---------------\n");
        for (int j = 0; j < len; j++) if (bc[j] >= 0) printf("%4c", (char)j); printf("\n");
        for (int j = 0; j < len; j++) if (bc[j] >= 0) printf("%4d", bc[j]); printf("\n\n");
    }

    void onGSTable(const int* gs, int len, const String& pattern) override {
        printf("-- gs[] Table ---------------\n");
        moPrintIndexLine(len);
        moPrintString(pattern); printf("\n");
        for (int j = 0; j < len; j++) printf("%4d", gs[j]);
        printf("\n\n");
    }

    void onKRProgress(const String& text, const String& pattern, size_t k, long long hashP, long long hashT) override {
        printf("\n-- Step %2d (k=%zu) --\n", ++step, k);
        size_t n = text.size();
        moPrintIndexLine(n);
        moPrintString(text); printf("\n");
        for (size_t t = 0; t < k; t++) printf("%4c", ' '); moPrintString(pattern); printf("\n");
        printf("hashP=%lld, hashT=%lld\n", hashP, hashT);
    }

    void onPause() override { getchar(); }
};

struct NoopMatchObserver : MatchObserver {};

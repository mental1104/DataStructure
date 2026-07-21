#ifndef __DSA_COMMON
#define __DSA_COMMON

#include <ctime>
#include <cstring>
#include <climits>
#include <cassert>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

inline void sleep_seconds(unsigned int seconds) {
    // 跨平台 sleep，避免直接依赖 POSIX 的 sleep() 在 Windows 下缺失
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

inline void clear_screen() {
    if (std::system("clear") == -1) {
        // ignore errors; clear is best-effort
    }
}

using size_type = unsigned;
using Rank = int;
using U = unsigned int;

const int DEFAULT_CAPACITY = 1024;

/* Fundamental Operation */
template<typename T>
void swap(T& ls, T& rs){
    T temp = ls;
    ls = rs;
    rs = temp;
}

template<typename T>
T min(T ls, T rs){
    if(ls < rs) return ls;
    else        return rs;
}


template<typename T>
T max(T ls, T rs){
    if(ls < rs) return rs;
    else        return ls;
}

#endif

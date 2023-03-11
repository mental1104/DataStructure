#pragma once

#include <ctime>
#include <unistd.h>

using size_type = unsigned;
using Rank = int;
using U = unsigned int;

const int DEFAULT_CAPACITY = 2;

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
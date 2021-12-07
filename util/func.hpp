#pragma once 

template<typename T> struct Print{
    virtual void operator() (T& e) { printf("%d ", e); }
};
#pragma once 

long long facI ( int n ) { long long f = 1; while ( n > 1 ) f *= n--; return f; }

template<typename T> struct Double{
    virtual void operator()(T& e) {  e*=2; }
};

template<typename T> struct Increment{
    virtual void operator()(T& e) {  e+=1; }
};

template<typename T> struct Decrement{
    virtual void operator()(T& e) {  e-=1; }
};
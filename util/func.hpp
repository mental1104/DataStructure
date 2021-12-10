#pragma once 

template<typename T> struct Double{
    virtual void operator()(T& e) {  e*=2; }
};

template<typename T> struct Increment{
    virtual void operator()(T& e) {  e+=1; }
};

template<typename T> struct Decrement{
    virtual void operator()(T& e) {  e-=1; }
};
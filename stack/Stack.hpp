#pragma once

#include "../def.hpp"

template<typename T> 
class Stack: public Vector<T> {
public:
    void push(T const& e)   {  this->insert(this->size(), e); }
    T pop()                 {   return this->remove(this->size() - 1);  }
    T& top()                {   return  (*this)[this->size()-1];    }
};


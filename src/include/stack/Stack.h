#ifndef __DSA_STACK
#define __DSA_STACK

#include "utils.h"
#include "Vector.h"

template<typename T> 
class Stack: public Vector<T> {
public:
    void push(T const& e)   {  this->insert(this->size(), e); }
    T pop()  {
        if (this->size() > 0) {
            return this->remove(this->size() - 1);  
        } else {
            throw std::out_of_range("Stack is empty");
        }
    }
    T& top() {   
        if (this->size() > 0) {
            return  (*this)[this->size()-1];
        } else {
            throw std::out_of_range("Stack is empty");
        }
    }
};

#endif
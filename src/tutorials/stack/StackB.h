#ifndef __DSA_STACKB
#define __DSA_STACKB

#include "utils.h"
#include "List.h"

template<typename T>
class Stack : public List<T> {
public:
    void push(T const& e) {     this->insertAsLast(e);    }
    T pop() {     
        if (this->size() > 0) {
            return this->remove(this->last()); 
        } else {
            throw std::out_of_range("Stack is empty");
        }
    }
    T& top() {     
        if (this->size() > 0) {
            return this->last()->data;
        } else {
            throw std::out_of_range("Stack is empty");
        }
            
    }
};

#endif
#pragma once

#include "../def.hpp"  

template<typename T>
class Stack : public List<T> {
public:
    void push(T const& e) {     this->insertAsLast(e);    }
    T pop()               {     return this->remove(this->last());    }
    T& top()              {     return this->last()->data;    }
};

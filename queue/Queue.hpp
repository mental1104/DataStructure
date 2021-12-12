#pragma once
#include "../def.hpp"

template<typename T> 
class Queue : public List<T> {
public:
    void enqueue (T const& e)   { this->insertAsLast(e); }
    T dequeue()                 { return this->remove(this->first()); }
    T& front()                  { return this->first()->data; }
};
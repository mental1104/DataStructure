/*
 * @Date: 2025-01-24 13:55:48
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2025-01-26 16:31:25
 */
#ifndef __DSA_QUEUE
#define __DSA_QUEUE


#include "utils.h"
#include "List.h"

template<typename T> 
class Queue : public List<T> {
public:
    void enqueue (T const& e)   { this->insertAsLast(e); }
    T dequeue() { 
        if (this->empty()) {
            throw std::runtime_error("Dequeue called on an empty queue.");
        }
        return this->remove(this->first()); 
    }
    T& front() {
        if (this->empty()) { // 检查队列是否为空
            throw std::runtime_error("Front called on an empty queue.");
        } 
        return this->first()->data; 
    }
};

#endif
#pragma once

#include "utils.h"

template <typename T>
class StringST {
public:
    using mapped_type = T;

    virtual ~StringST() = default;

    bool empty() const noexcept { return size_ == 0; }
    int size() const noexcept { return static_cast<int>(size_); }

protected:
    size_type size_{0};
};

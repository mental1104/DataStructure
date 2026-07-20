#pragma once

#include "utils.h"

template <typename T>
class StringST {
public:
    using mapped_type = T;

    virtual ~StringST() = default;

    bool empty() const noexcept { return size_ == 0; }
    size_type size() const noexcept { return size_; }

protected:
    size_type size_{0};
};

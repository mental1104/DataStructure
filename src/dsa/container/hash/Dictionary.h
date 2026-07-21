#ifndef DSA_CONTAINER_HASH_DICTIONARY_H
#define DSA_CONTAINER_HASH_DICTIONARY_H

#include <cstddef>

namespace dsa {
namespace container {

// 面向旧教学门面的最小字典抽象；具体容器仍可同时提供 STL 风格接口。
template<typename Key, typename T>
class Dictionary {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::size_t size_type;

    virtual ~Dictionary() {}
    virtual size_type size() const = 0;
    virtual bool put(const key_type& key, const mapped_type& value) = 0;
    virtual mapped_type* get(const key_type& key) = 0;
    virtual const mapped_type* get(const key_type& key) const = 0;
    virtual bool remove(const key_type& key) = 0;
};

} // namespace container
} // namespace dsa

#endif

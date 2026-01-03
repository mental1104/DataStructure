#ifndef __DSA_STRING
#define __DSA_STRING

#include "utils.h"
#include <cstring>

class String {
public:
    using value_type = char;

    String();//默认构造函数
    String(const char* s);//C风格字符串的构造函数
    String(char c);
    String(const String& );//拷贝构造函数
    String(const char* s, size_type k);//从位置s开始构造k个字符

    ~String() { delete[] data_; data_ = nullptr; end_ = nullptr;}//析构函数
    String& operator=(const String&);//拷贝赋值运算符

    const char& front() const;//第一个元素
    const char& back()  const;//最后一个元素
    bool check(size_type i) const;//下标越界检查
    const char* c_str() const;//C风格字符串

    size_type size() const {  return static_cast<size_type>(end_ - data_); }//大小
    bool empty() const {    return end_ == data_;   }//判空

    char charAt(size_type i)/* 返回对应字符前进行下标检查 */{   if(check(i))  return (*this)[i]; return '\0';}
    String substr(size_type i, size_type k = -1);//返回从i开始的k个字符的子串
    String prefix(size_type k);//返回前缀
    String suffix(size_type k);//返回后缀
    bool equal(const String& rhs)/* 判等 */ {   return *this == rhs;  }
    String& concat(const String& rhs);//字符串拼接
    //int indexOf(const String& rhs);//子串匹配

    char& operator[](size_type r);//重载下标运算符
    char operator[] (size_type) const;
    bool operator==(const String& rhs);//重载判等运算符
    bool operator!=(const String& rhs) {    return !(*this == rhs);     }
    String operator+(const String& rhs);
    String operator+(char rhs);
    bool operator<(const String& rhs) const; 
    template<typename VST> void traverse(VST&& visit);
    void traverse(void (*visit)(char&));

    char* begin() { return data_; }
    const char* begin() const { return data_; }
    char* end() { return end_; }
    const char* end() const { return end_; }

private:
    char* data_;
    char* end_;
};

String::String():data_(new char[1])
{
    *data_ = '\0';
    end_ = data_;
}

String::String(char c): data_(new char[2]) {
    data_[0] = c;
    data_[1] = '\0';
    end_ = data_ + 1;  // ✅ 正确设置 end_ 指向字符串末尾
}

String::String(const char* s){
    size_type len = static_cast<size_type>(std::strlen(s));
    data_ = new char[len + 1];
    std::memcpy(data_, s, len + 1);
    end_ = data_ + len;
}

String::String(const String& rhs){
    size_type len = static_cast<size_type>(std::strlen(rhs.c_str()));
    data_ = new char[len + 1];
    std::memcpy(data_, rhs.c_str(), len + 1);
    end_ = data_ + len;
}

String::String(const char* s, size_type k){
    data_ = new char[k+1];
    size_type i;
    for(i = 0; i < k; i++){
        data_[i] = s[i];
    }
    data_[i] = '\0';
    end_ = &data_[i];
}

const char& String::front() const {
    return *data_;
}

const char& String::back() const {
    return *(end_ - 1);
}

char& String::operator[](size_type r){
    return data_[r];
}

char String::operator[](size_type r) const {
    return data_[r];
}

const char* String::c_str() const{
    return data_;
}

String& String::operator=(const String& rhs){
    if(&rhs != this){
        const char* temp = rhs.c_str();
        delete[] data_;
        size_type len = static_cast<size_type>(std::strlen(temp));
        data_ = new char[len + 1];
        std::memcpy(data_, temp, len + 1);
        end_ = data_ + len;
    }
    return *this;
}

bool String::check(size_type i) const
{
    if (i >= this->size()) 
        return false;
    return true;
}

String String::substr(size_type i, size_type k){
    String ret;
    if(check(i)){
        size_type comp = this->size() - i;

        size_type size = (k<comp)?k:comp;
        //printf("size:  %u\n", size);
        String str(data_+i, size);
        ret = str;
    }
    return ret;
}

bool String::operator==(const String& rhs){
    if(!strcmp(this->data_, rhs.data_))
        return true;
    return false;
}

String String::prefix(size_type k){
    if(check(k))
        return String(this->data_, k);
    return String(*this);
}

String String::suffix(size_type k){
    if(check(k))
        return String(this->end_ - k, k);
    return String(*this);
}

String& String::concat(const String& rhs){
    size_type l = this->size();
    size_type r = rhs.size();
    size_type sum = l+r;
    char* n = new char[sum+1];
    size_type i;
    for(i = 0; i < l; i++){
        *(n+i) = (*this)[i];
    }

    for(size_type j = 0; i < sum; i++, j++){
        *(n+i) = *(rhs.data_ +j);
    }

    *(n+i) = '\0';
    end_ = n+i;

    delete[] data_;
    data_ = n;
    return *this;
}

String String::operator+(const String& rhs){
    size_type l = this->size();
    size_type r = rhs.size();
    size_type sum = l+r;
    char* n = new char[sum+1];
    size_type i;
    for(i = 0; i < l; i++){
        *(n+i) = (*this)[i];
    }

    for(size_type j = 0; i < sum; i++, j++){
        *(n+i) = *(rhs.data_ +j);
    }

    *(n+i) = '\0';
    String ret(n);
    delete[] n;
    return ret;
}

String String::operator+(char rhs){
    
    char* n = new char[this->size()+2];
    size_type i;
    for(i = 0; i < this->size(); i++){
        *(n+i) = (*this)[i];
    }
    *(n+i) = rhs;
    i++;
    *(n+i) = '\0';

    String ret(n);
    delete[] n;

    return ret;
}

bool String::operator<(const String& rhs) const { 
    if(strcmp(this->data_, rhs.data_) < 0) 
        return true;
    else 
        return false;
}


template<typename VST>
void String::traverse(VST&& visit){
    char* ptr = data_;
    while(ptr != end_){
        visit(*ptr);
        ++ptr;
    }
}

void String::traverse(void (*visit)(char&)){
    char* ptr = data_;
    while(ptr != end_){
        visit(*ptr);
        ++ptr;
    }
}

#endif

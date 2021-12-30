#pragma once
#include "../def.hpp"

class String {
public:
    String();//默认构造函数
    String(const char* s);//C风格字符串的构造函数
    String(const String& );//拷贝构造函数
    String(const char* s, size_type k);//从位置s开始构造k个字符

    ~String() { delete[] data_; data_ = nullptr; end_ = nullptr;}//析构函数
    String& operator=(const String&);//拷贝赋值运算符

    const char& front() const;//第一个元素
    const char& back()  const;//最后一个元素
    bool check(int i) const;//下标越界检查
    const char* c_str() const;//C风格字符串

    size_type size() const {  return end_ - data_; }//大小
    bool empty() const {    return end_ == data_;   }//判空

    char charAt(size_type i)/* 返回对应字符前进行下标检查 */{   if(check(i))  return (*this)[i]; return '\0';}
    String substr(size_type i, size_type k);//返回从i开始的k个字符的子串
    String prefix(size_type k);//返回前缀
    String suffix(size_type k);//返回后缀
    bool equal(const String& rhs)/* 判等 */ {   return *this == rhs;  }
    String& concat(const String& rhs);//字符串拼接
    //int indexOf(const String& rhs);//子串匹配

    char& operator[](size_type r);//重载下标运算符
    bool operator==(const String& rhs);//重载判等运算符
    bool operator!=(const String& rhs) {    return !(*this == rhs);     }
    String& operator+(const String& rhs);
    template<typename VST> void traverse(VST&& visit);
    void traverse(void (*visit)(char&));

private:
    char* data_;
    char* end_;
};

String::String():data_(new char[1])
{
    *data_ = '\0';
    end_ = data_;
}

String::String(const char* s):data_(new char[strlen(s)+1]){
    strcpy(data_, s);
    end_ = data_ + strlen(s);
}

String::String(const String& rhs){
    int size = strlen(rhs.c_str());
    data_ = new char[size+1];
    strcpy(data_, rhs.c_str());
    end_ = data_ + strlen(rhs.c_str());
}

String::String(const char* s, size_type k){
    data_ = new char[k+1];
    int i;
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

const char* String::c_str() const{
    return data_;
}

String& String::operator=(const String& rhs){
    if(&rhs != this){
        const char* temp = rhs.c_str();
        delete[] data_;
        data_ = new char[strlen(temp)+1];
        strcpy(data_, temp);
        end_ = data_ + strlen(temp);
    }
    return *this;
}

bool String::check(int i) const
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
        printf("size:  %u\n", size);
        String str(data_+i, size);
        ret = str;
    }
    return std::move(ret);
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
    int i;
    for(i = 0; i < l; i++){
        *(n+i) = (*this)[i];
    }

    for(int j = 0; j < r && i < sum; i++, j++){
        *(n+i) = *(rhs.data_ +j);
    }

    *(n+i) = '\0';
    end_ = n+i;

    delete[] data_;
    data_ = n;
    return *this;
}

String& String::operator+(const String& rhs){
    return concat(rhs);
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
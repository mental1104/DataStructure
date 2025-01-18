
#ifndef __DSA_VECTOR
#define __DSA_VECTOR

#include "utils.h"
#include "Fib.h"

enum class Sort {
    BubbleSort,
    SelectionSort,
    InsertionSort,
    ShellSort,
    MergeSortA,
    MergeSortB,
    QuickSort,
    Quick3way,
    QuickSortB,
    HeapSort,
};

template<typename T>
class Vector{
protected:
    int _size;
    int _capacity;
    T* _elem;
    int heap;

    void copyFrom(T const* A, Rank lo, Rank hi);
    void expand();//扩容
    void shrink();//缩容

public:
    //构造函数
    Vector(int c = DEFAULT_CAPACITY, int s = 0, T v = T()){
        _capacity = c;
        _elem = new T[_capacity];
        for(_size = 0; _size < s; _elem[_size++] = v);
    }
    Vector(T const* A, Rank n){ copyFrom(A, 0, n); }
    Vector(T const* A, Rank lo, Rank hi){   copyFrom(A, lo, hi);    }
    Vector(Vector<T> const& V) {    copyFrom(V._elem, 0, V._size);   }//copy constructor
    Vector(Vector<T> const& V, Rank lo, Rank hi) {  copyFrom(V._elem, lo, hi); }
    //析构函数
    ~Vector()   {   delete []_elem;   }
    
    Vector<T> & operator=(Vector<T> const&);
    T& operator[] (Rank r) const;

    Rank size() const { return _size; }
    bool empty() const { return !_size; }
    Rank capacity() const { return _capacity; }

    struct iterator;
    iterator begin();
    iterator end();

    int disordered()  const;
    Rank find(T const& e) const { return find(e, 0, _size); }
    Rank find(T const& e, Rank lo, Rank hi) const;
    Rank binSearch (T* A, T const& e, Rank lo, Rank hi) const;
    Rank fibSearch (T* A, T const& e, Rank lo, Rank hi) const;
    Rank search(T const& e) const { return (0 >= _size)?-1:search(e, 0, _size); }
    Rank search(T const& e, Rank lo, Rank hi) const;

    T remove(Rank r);//常规
    int remove(Rank lo, Rank hi);//常规
    Rank insert(Rank r, T const& e);//常规
    Rank insert(T const& e) {   return insert(_size, e);  }//常规
    T majEleCandidate();
    void range(int k);
    void unsort(Rank lo, Rank hi);
    void unsort() { unsort(0, _size); }
    int deduplicate();
    int uniquify();

    void traverse(void(*)(T&));//常规
    template<typename VST> void traverse(VST&&);//常规
};


template<typename T>
void Vector<T>::copyFrom(T const* A, Rank lo, Rank hi){
    _elem = new T[_capacity = 2 * (hi - lo)];
    _size = 0;
    while(lo < hi)
        _elem[_size++] = A[lo++];
}

template<typename T>
Vector<T>& 
Vector<T>::operator=(Vector<T> const& V){
    if( _elem ) delete []_elem;
    copyFrom(V._elem, 0, V.size());
    return *this;
}

template<typename T>
void Vector<T>::expand(){
    if(_size < _capacity)  return;
    if(_capacity < DEFAULT_CAPACITY) _capacity = DEFAULT_CAPACITY;
    T* oldElem = _elem;
    _capacity = _capacity << 1;
    
    _elem = new T[_capacity];
    
    for(int i = 0; i < _size; i++)
        _elem[i] = oldElem[i];
    delete []oldElem;
}

template<typename T>
void Vector<T>::shrink(){
    if(_capacity < DEFAULT_CAPACITY << 1) return;
    if(_size << 2 > _capacity) return;
    T* oldElem = _elem;
    _elem = new T[_capacity >>= 1];
    for(int i = 0; i < _size; i++)
        _elem[i] = oldElem[i];
    delete []oldElem;
}

template<typename T>
T& Vector<T>::operator[](Rank r) const{
    return _elem[r];
}

template<typename T> 
void Vector<T>::unsort(Rank lo, Rank hi){
    T* V = _elem + lo;
    for(Rank i = hi - lo; i > 0; i--)
        swap(V[i-1], V[dice(2021)%i]);
}

template<typename T>
Rank Vector<T>::find(T const& e, Rank lo, Rank hi) const{
    while((lo < hi--) && (e!=_elem[hi]))
        ;
    return hi;
}

template<typename T>
Rank Vector<T>::insert(Rank r, T const& e){
    expand();
    for(int i = _size; i > r; i--)
        _elem[i] = _elem[i-1];
    _elem[r] = e;
    _size++;
    return r;
}

template<typename T>
int Vector<T>::remove(Rank lo, Rank hi){
    if(lo == hi) return 0;
    while(hi < _size) _elem[lo++] = _elem[hi++];
    _size = lo;
    shrink();
    return hi-lo;
}

template<typename T>
T Vector<T>::remove(Rank r){
    T e = _elem[r];
    remove(r, r+1);
    return e;
}

template<typename T>
int Vector<T>::deduplicate(){
    int oldSize = _size;
    Rank i = 1;
    while(i < _size)
        (find(_elem[i], 0, i)<0) ? i++:remove(i);
    return oldSize - _size;
}

template<typename T> 
void Vector<T>::traverse(void (*visit)(T&)){
    for(int i = 0; i <_size; i ++)
        visit(_elem[i]);
}

template<typename T>
template<typename VST>
void Vector<T>::traverse(VST&& visit){
    for(int i = 0; i < _size; i++)
        visit(_elem[i]);
}

template<typename T>
int Vector<T>::disordered() const {
    int n = 0;
    for(int i = 1; i < _size; i++)
        if(_elem[i-1] > _elem[i]) n++;
    return n;
}

template<typename T>
int Vector<T>::uniquify(){
    Rank i = 0, j = 0;
    while( ++j < _size)
        if(_elem[i]!=_elem[j])
            _elem[++i] = _elem[j];
    _size = ++i; 
    shrink();
    return j - i;
}

template<typename T>
Rank Vector<T>::search(T const& e, Rank lo, Rank hi) const{
    return binSearch(_elem, e, lo, hi);
}

template<typename T> 
Rank Vector<T>::binSearch (T* A, T const& e, Rank lo, Rank hi) const{
    //printf("BinSearch\n");
    while(lo < hi){
        Rank mi = (lo + hi) >> 1;
        (e < A[mi])? hi = mi:lo = mi+1;
    }
    return --lo;
}

template<typename T>
Rank Vector<T>::fibSearch (T* A, T const& e, Rank lo, Rank hi) const{
    //printf ( "FIB search (B)\n" );
    for( Fib fib ( hi - lo ); lo < hi;  ) { //Fib数列制表备查
        while( hi - lo < fib.get() ) fib.prev(); //自后向前顺序查找（分摊O(1)）
        Rank mi = lo + fib.get() - 1; //确定形如Fib(k) - 1的轴点
        ( e < A[mi] ) ? hi = mi : lo = mi + 1; //比较后确定深入前半段[lo, mi)或后半段(mi, hi)
    } //成功查找不能提前终止
    return --lo; 
}


//众数定义为序列中一半以上的数（刚好一半不行）
template<typename T>
T Vector<T>::majEleCandidate(){
    T maj;

    for(int c = 0, i = 0; i < size(); i++)
        if(0 == c){
            maj = _elem[i];
            c = 1;
        } else {
            maj == _elem[i]? c++ : c--;
        }
    return maj;
}

template<typename T>
void Vector<T>::range(int k){
    int r = size()/k;
    Vector<int> vec{k-1, k-1, -1};
    Vector<int> c{k-1, k-1, 0};
    for(int i = 0; i < size(); i++){
        Rank index = c.find(0);
        if(index >= 0){
            vec[index] = _elem[i];
            c[index] = 1;
        } else {
            Rank v = vec.find(_elem[i]);
            if(v!=-1){
                c[v]++;
            } else {
                for(int j = 0; j < c.size(); j++)
                    c[j]--;
            }
        }
    }
    vec.deduplicate();
    for(int j = 0; j < vec.size(); j++){
        int occurrence = 0; //maj在A[]中出现的次数
        for ( int i = 0; i < size(); i++ ) //逐一遍历A[]的各个元素
            if (vec[j] == _elem[i] ) occurrence++; //每遇到一次maj，均更新计数器
        //if(k * occurrence > size()) print(vec[j]);
    }
    //printf("\n");
}

template<typename T>
struct Vector<T>::iterator {
    T* cur;
    
    explicit iterator(T* rhs)
        : cur{rhs} {}
    
    bool operator!=(const iterator& other) {
        return cur != other.cur;
    }

    T& operator*() {
        return *cur;
    }

    iterator& operator++(){
        cur++;
        return *this;
    }
};

template<typename T>
typename Vector<T>::iterator
Vector<T>::begin() {
    return iterator{_elem};
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::end() {
    return iterator{_elem + _size};
}

#endif
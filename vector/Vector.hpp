#pragma once

#include "../def.hpp"

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
public:
    int _size;
    int _capacity;
    T* _elem;
    int heap;

    void copyFrom(T const* A, Rank lo, Rank hi);
    void expand();//扩容
    void shrink();//缩容

    bool bubble(Rank lo, Rank hi);
    void bubbleSort(Rank lo, Rank hi);
    void selectionSort(Rank lo, Rank hi);

    void insertionSort(Rank lo, Rank hi);
    void shellSort(Rank lo, Rank hi);

    void topDown(T* aux, Rank lo, Rank hi);
    void merge(T* aux, Rank lo, Rank mid, Rank hi);
    void mergeSortA(Rank lo, Rank hi);
    void mergeSortB(Rank lo, Rank hi);

    Rank partition(Rank lo, Rank hi);//排序
    void quick(Rank lo, Rank hi);
    void quickSort(Rank lo, Rank hi);//排序

    void quickB(Rank lo, Rank hi);
    void quickSortB(Rank lo, Rank hi);

    void quick3way(Rank lo, Rank hi);

    void sink(Rank k);
    void heapSort(Rank lo, Rank hi);

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

    double sort(Sort method = Sort::QuickSort) {   return sort(0, _size, method); }
    double sort(Rank lo, Rank hi, Sort method);
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

template<typename T>
double Vector<T>::sort(Rank lo, Rank hi, Sort method){
    clock_t start, end;
    start = clock();
    switch(method){
        case Sort::BubbleSort: 
            //printf("BubbleSort.\n");
            bubbleSort(lo, hi); 
            break;
        case Sort::SelectionSort: 
            //printf("SelectionSort.\n");
            selectionSort(lo, hi); 
            break;
        case Sort::InsertionSort:
            //printf("InsertionSort.\n");
            insertionSort(lo, hi);
            break;
        case Sort::ShellSort:
            //printf("ShellSort.\n"); 
            shellSort(lo, hi); 
            break;
        case Sort::MergeSortA:
            //printf("MergeSort(Top-down).\n"); 
            mergeSortA(lo, hi); 
            break;
        case Sort::MergeSortB:
            //printf("MergeSort(Bottom-Up).\n"); 
            mergeSortB(lo, hi); 
            break;
        case Sort::Quick3way: 
            //printf("QuickSort(3-way).\n");
            quick3way(lo, hi);
            break;
        case Sort::QuickSortB:
            //printf("QuickSortB.\n");
            quickSortB(lo, hi);
            break;
        case Sort::HeapSort:
            //printf("HeapSort.\n");
            heapSort(lo, hi);
        default:
            //printf("QuickSort.\n");
            quickSort(lo, hi);
            break;
    }
    end = clock();
    return double(end-start)/CLOCKS_PER_SEC;
}

template<typename T>
bool Vector<T>::bubble(Rank lo, Rank hi){
    bool sorted = true;
    while(++lo < hi)
        if(_elem[lo - 1] > _elem[lo]){
            sorted = false;
            swap(_elem[lo-1], _elem[lo]);
        }
    return sorted;
}

template<typename T>
void Vector<T>::bubbleSort(Rank lo, Rank hi){
    while(!bubble(lo, hi--));
}

//From Algorithm 2.1 SelectionSort
template<typename T>
void Vector<T>::selectionSort(Rank lo, Rank hi){
    for(int i = lo; i < hi; i++){
        int min = i;
        for(int j = i+1; j < hi; j++)
            if(_elem[j] < _elem[min]) 
                min = j;
            swap(_elem[i], _elem[min]);
    }
    return;
}

//From Algorithm 2.2 insertionSort
template<typename T>
void Vector<T>::insertionSort(Rank lo, Rank hi){
    for(int i = lo+1; i < hi; i++){
        for(int j = i; j > 0 && _elem[j] < _elem[j-1]; j--)
            swap(_elem[j], _elem[j-1]);
    }
}

//From Algorithm 2.3 shellSort
template<typename T>
void Vector<T>::shellSort(Rank lo, Rank hi){
    int N = hi - lo;
    int h = 1;
    while(h < N/3) h = 3*h + 1;//1, 4, 13, 40, 121, 364, 1093, ...
    while(h >= 1){
        for(int i = lo+h; i < hi; i++){
            for(int j = i; j >= lo+h && _elem[j] < _elem[j-h]; j-=h)
                swap(_elem[j], _elem[j-h]);
        }
        h = h/3;
    }
}

template<typename T>
void Vector<T>::topDown(T* aux, Rank lo, Rank hi){
    if(lo >= hi) return;
    Rank mid = lo + (hi-lo)/2;
    topDown(aux, lo, mid);
    topDown(aux, mid+1, hi);
    merge(aux, lo, mid, hi);
}

template<typename T>
void Vector<T>::merge(T* aux, Rank lo, Rank mid, Rank hi){
    Rank i = lo, j = mid + 1;

    for(Rank k = lo; k <= hi; k++)
        aux[k] = _elem[k];
    
    for(Rank k = lo; k <= hi; k++)
        if      (i > mid)           _elem[k] = aux[j++];//当左数组耗尽时
        else if (j > hi)            _elem[k] = aux[i++];//当右数组耗尽时
        else if (aux[j] < aux[i])   _elem[k] = aux[j++];//比较两个数组队列头元素，取小者
        else                        _elem[k] = aux[i++];   
}


//From Algorithm 2.4 Top-down mergeSort
template<typename T>
void Vector<T>::mergeSortA(Rank lo, Rank hi){
    T* aux = new T[hi-lo];
    topDown(aux, lo, hi-1);
    delete[] aux;
}


template<typename T>
void Vector<T>::mergeSortB(Rank lo, Rank hi){
    int N = hi - lo;
    T* aux = new T[N];
    for(int sz = 1; sz < N; sz = sz+sz)
        for(int lo = 0; lo < N-sz; lo += sz+sz){
            merge(aux, lo, lo+sz-1, min(lo+sz+sz-1, N-1));
        }
    delete[] aux;
}

template<typename T>
void Vector<T>::quickSort(Rank lo, Rank hi){
    //unsort(lo, hi);
    quick(lo, hi-1);
}

template<typename T>
void Vector<T>::quick(Rank lo, Rank hi){
    if (lo >= hi) return;
    Rank j = partition(lo, hi);
    quick(lo, j-1);
    quick(j+1, hi);
}

template<typename T>
Rank Vector<T>::partition(Rank lo, Rank hi){
    Rank i = lo, j = hi+1;
    T v = _elem[lo];
    while(true){
        while(_elem[++i] < v)   if(i == hi) break;
        while(v < _elem[--j])   if(j == lo) break;
        if(i >= j) break;
        swap(_elem[i], _elem[j]);
    }
    swap(_elem[lo],_elem[j]);
    return j;
}

template<typename T>
void Vector<T>::quick3way(Rank lo, Rank hi){
    if(lo >= hi) return;
    Rank lt = lo, i = lo+1, gt = hi-1;
    T v = _elem[lo];
    while(i <= gt){
        if      (_elem[i] < v) swap(_elem[lt++], _elem[i++]);
        else if (_elem[i] > v) swap(_elem[i], _elem[gt--]);
        else                   i++;
    }
    quick3way(lo, lt-1);
    quick3way(gt+1, hi);
}

template<typename T>
void Vector<T>::quickSortB(Rank lo, Rank hi){
    //unsort(lo, hi);
    quickB(lo, hi-1);
}

template<typename T>
void Vector<T>::quickB(Rank lo, Rank hi){
    if (lo + 15 >= hi) {
        insertionSort(lo, hi);
        return;
    }
    Rank j = partition(lo, hi);
    quickB(lo, j-1);
    quickB(j+1, hi);
}

template<typename T>
void Vector<T>::sink(Rank k){
    while(2*k <= heap){
        int j = 2*k;
        if(j < heap && _elem[j] < _elem[j+1]) j++;
        if(!_elem[k] < _elem[j]) break;
        swap(_elem[k], _elem[j]);
        k = j;
    }
}

template<typename T>
void Vector<T>::heapSort(Rank lo, Rank hi){
    heap = hi - lo - 1;
    for(int k = heap/2; k >= 1; k--)
        sink(k);
    while(heap > 1){
        swap(_elem[1], _elem[heap--]);
        sink(1);
    }
}
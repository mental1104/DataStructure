#include "../def.hpp"

int main(){
    Vector<int> left;
    left.insert(6);
    left.insert(13);
    left.insert(12);
    left.insert(17);
    Vector<int> right;
    right.insert(10);
    right.insert(8);
    right.insert(15);

    PQ_ComplHeap<int> LeftComplHeap(left);
    PQ_ComplHeap<int> RightComplHeap(right);
    print(LeftComplHeap);
    print(RightComplHeap);
    printf("Merge two heaps，using complete binary heap(O(nlogm)\n");
    while(!RightComplHeap.empty())
        LeftComplHeap.insert(RightComplHeap.delMax());
    print(LeftComplHeap);
    print(RightComplHeap);
    printf("------------------------------------------------------\n");
    PQ_LeftHeap<int> LeftLeftHeap(left);
    PQ_LeftHeap<int> RightLeftHeap(right);
    print(LeftLeftHeap);
    print(RightLeftHeap);
    printf("Merge two heaps，using Left heap(O(logn)\n");
    LeftLeftHeap.merge(RightLeftHeap);
    print(LeftLeftHeap);
    print(RightLeftHeap);
    printf("Done.\n");
    printf("-------------Now delete...---------------\n");
    while(!LeftLeftHeap.empty()){
        LeftLeftHeap.delMax();
        print(LeftLeftHeap);
    }
    return 0;
}
#pragma once

template <typename T>
inline void VectorSortImpl::selectionSort(Vector<T> &container, Rank lo, Rank hi)
{
    for(int i = lo; i < hi; i++){
        int min = i;
        for(int j = i+1; j < hi; j++){
            if(container[j] < container[min]) {
                min = j;
            }
        }
        swap(container[i], container[min]);
    }
}

template <typename T>
inline ListNode<T> *ListSortImpl::selectMax(ListNode<T> *p, int n)
{
    ListNode<T>* max = p;
    for(ListNode<T>* cur = p; 1 < n; n--){
        cur = cur->succ;
        if(!(cur->data < max->data))
            max = cur;
    }
    return max;
}

template <typename T>
inline void ListSortImpl::selectionSort(List<T>& container)
{
    ListNode<T> *p = container.first();
    int n = container.size();
    ListNode<T>* head = p->pred;
    ListNode<T>* tail = p;
    for(int i = 0; i < n; i++)
        tail = tail->succ;
    while(1 < n){
        ListNode<T>* max = selectMax(head->succ, n);
        container.insertB(tail, container.remove(max));
        tail = tail->pred;
        n--;
    }
}

template <typename T>
inline void ListSortImpl::radixSort(List<T> &container)
{
    ListNode<T>* p = container.first();
    int n = container.size();
    ListNode<T>* head = p->pred;
    ListNode<T>* tail = p;
    for(int i = 0; i < n; i++) tail = tail->succ;
    for(U radixBit = 0x1; radixBit && (p = head); radixBit <<= 1)
        for(int i = 0; i < n; i++){
            if(radixBit & U(p->succ->data))
                tail->insertAsPred(container.remove(p->succ));
            else
                p = p->succ;
        }
}

#ifndef __DSA_LISTNODE
#define __DSA_LISTNODE

#include <utility>

/// 教学链表节点；哨兵沿用默认构造 T 的旧存储模型。
template<typename T>
struct ListNode {
    T data;
    ListNode<T>* pred;
    ListNode<T>* succ;

    /// 构造教学哨兵，因此仍要求 T 可默认构造。
    ListNode()
        : data(), pred(nullptr), succ(nullptr) {
    }

    /// 复制构造业务节点并设置前驱、后继链接。
    ListNode(
        const T& value,
        ListNode<T>* previous = nullptr,
        ListNode<T>* next = nullptr
    ) : data(value), pred(previous), succ(next) {
    }

    /// 移动构造业务节点并设置前驱、后继链接。
    ListNode(
        T&& value,
        ListNode<T>* previous = nullptr,
        ListNode<T>* next = nullptr
    ) : data(std::move(value)), pred(previous), succ(next) {
    }

    ListNode<T>* insertAsPred(T const& e);
    ListNode<T>* insertAsSucc(T const& e);
};

/// 在当前节点前复制插入业务节点；该兼容接口仍由节点自身维护链接。
template<typename T>
ListNode<T>* ListNode<T>::insertAsPred(T const& e) {
    ListNode<T>* node = new ListNode<T>(e, pred, this);
    pred->succ = node;
    pred = node;
    return node;
}

/// 在当前节点后复制插入业务节点；该兼容接口仍由节点自身维护链接。
template<typename T>
ListNode<T>* ListNode<T>::insertAsSucc(T const& e) {
    ListNode<T>* node = new ListNode<T>(e, this, succ);
    succ->pred = node;
    succ = node;
    return node;
}

#endif

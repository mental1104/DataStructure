#ifndef DSA_ALGORITHM_SORT_H
#define DSA_ALGORITHM_SORT_H

#include <cstddef>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dsa {
namespace algorithm {

/// 标识仓库保留的排序策略；容器 facade 与 iterator-first 算法共同使用该枚举。
enum class SortStrategy {
    BubbleSort,
    SelectionSort,
    InsertionSort,
    ShellSort,
    MergeSort,
    MergeSortB,
    QuickSort,
    Quick3way,
    QuickSortB,
    HeapSort,
    RadixSort
};

namespace sort_detail {

/// 判断一个枚举值是否属于公开排序策略，非法强转值继续保持旧 facade 的空操作语义。
inline bool isKnownStrategy(SortStrategy strategy) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
    case SortStrategy::SelectionSort:
    case SortStrategy::InsertionSort:
    case SortStrategy::ShellSort:
    case SortStrategy::MergeSort:
    case SortStrategy::MergeSortB:
    case SortStrategy::QuickSort:
    case SortStrategy::Quick3way:
    case SortStrategy::QuickSortB:
    case SortStrategy::HeapSort:
    case SortStrategy::RadixSort:
        return true;
    }
    return false;
}

/// 将两个已排序的随机访问区间稳定合并；临时缓冲区只管理元素，不接触容器存储状态。
template<typename RandomIt, typename Compare>
void mergeRange(
    RandomIt first,
    RandomIt middle,
    RandomIt last,
    Compare compare,
    std::vector<typename std::iterator_traits<RandomIt>::value_type>& buffer
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    const Difference leftCount = middle - first;
    buffer.clear();
    for (RandomIt current = first; current != last; ++current)
        buffer.emplace_back(std::move(*current));

    std::size_t left = 0;
    std::size_t right = static_cast<std::size_t>(leftCount);
    const std::size_t leftEnd = right;
    const std::size_t rightEnd = buffer.size();
    RandomIt output = first;

    while (left < leftEnd && right < rightEnd) {
        if (compare(buffer[right], buffer[left]))
            *output++ = std::move(buffer[right++]);
        else
            *output++ = std::move(buffer[left++]);
    }
    while (left < leftEnd)
        *output++ = std::move(buffer[left++]);
    while (right < rightEnd)
        *output++ = std::move(buffer[right++]);
}

/// 递归拆分随机访问区间，并使用稳定合并完成自顶向下归并排序。
template<typename RandomIt, typename Compare>
void mergeSortTopDownImpl(
    RandomIt first,
    RandomIt last,
    Compare compare,
    std::vector<typename std::iterator_traits<RandomIt>::value_type>& buffer
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    const Difference count = last - first;
    if (count < 2)
        return;

    RandomIt middle = first + count / 2;
    mergeSortTopDownImpl(first, middle, compare, buffer);
    mergeSortTopDownImpl(middle, last, compare, buffer);

    if (!compare(*middle, *(middle - 1)))
        return;
    mergeRange(first, middle, last, compare, buffer);
}

/// 以末元素为固定枢轴执行二路划分，划分过程中不复制枢轴，兼容 move-only 元素。
template<typename RandomIt, typename Compare>
RandomIt partitionLastPivot(RandomIt first, RandomIt last, Compare compare) {
    RandomIt pivot = last - 1;
    RandomIt boundary = first;
    for (RandomIt current = first; current != pivot; ++current) {
        if (compare(*current, *pivot)) {
            if (current != boundary)
                std::iter_swap(current, boundary);
            ++boundary;
        }
    }
    std::iter_swap(boundary, pivot);
    return boundary;
}

/// 使用较小分区递归、较大分区尾循环，限制普通快速排序的递归深度。
template<typename RandomIt, typename Compare>
void quickSortImpl(RandomIt first, RandomIt last, Compare compare) {
    while (last - first > 1) {
        RandomIt pivot = partitionLastPivot(first, last, compare);
        if (pivot - first < last - (pivot + 1)) {
            quickSortImpl(first, pivot, compare);
            first = pivot + 1;
        } else {
            quickSortImpl(pivot + 1, last, compare);
            last = pivot;
        }
    }
}

/// 以末元素为固定枢轴完成三向划分，返回等值区间的左右边界。
template<typename RandomIt, typename Compare>
std::pair<RandomIt, RandomIt> partitionThreeWay(
    RandomIt first,
    RandomIt last,
    Compare compare
) {
    RandomIt pivot = last - 1;
    RandomIt less = first;
    RandomIt current = first;
    RandomIt greater = pivot;

    while (current != greater) {
        if (compare(*current, *pivot)) {
            std::iter_swap(current, less);
            ++current;
            ++less;
        } else if (compare(*pivot, *current)) {
            --greater;
            std::iter_swap(current, greater);
        } else {
            ++current;
        }
    }
    std::iter_swap(greater, pivot);
    return std::make_pair(less, greater + 1);
}

/// 对重复元素友好地递归排序三向划分后的左右区间。
template<typename RandomIt, typename Compare>
void quick3WaySortImpl(RandomIt first, RandomIt last, Compare compare) {
    while (last - first > 1) {
        const std::pair<RandomIt, RandomIt> equalRange =
            partitionThreeWay(first, last, compare);
        if (
            equalRange.first - first <
            last - equalRange.second
        ) {
            quick3WaySortImpl(first, equalRange.first, compare);
            first = equalRange.second;
        } else {
            quick3WaySortImpl(equalRange.second, last, compare);
            last = equalRange.first;
        }
    }
}

/// 通过首、中、尾三点排序选择枢轴，并将枢轴固定在尾部前一位置。
template<typename RandomIt, typename Compare>
RandomIt medianOfThreePivot(RandomIt first, RandomIt last, Compare compare) {
    RandomIt middle = first + (last - first) / 2;
    RandomIt tail = last - 1;

    if (compare(*middle, *first))
        std::iter_swap(middle, first);
    if (compare(*tail, *first))
        std::iter_swap(tail, first);
    if (compare(*tail, *middle))
        std::iter_swap(tail, middle);

    RandomIt pivot = tail - 1;
    std::iter_swap(middle, pivot);
    return pivot;
}

/// 使用三点取中枢轴完成双向扫描划分，供混合快速排序复用。
template<typename RandomIt, typename Compare>
RandomIt partitionMedianOfThree(RandomIt first, RandomIt last, Compare compare) {
    RandomIt pivot = medianOfThreePivot(first, last, compare);
    RandomIt left = first;
    RandomIt right = pivot;

    for (;;) {
        do {
            ++left;
        } while (compare(*left, *pivot));

        do {
            --right;
        } while (compare(*pivot, *right));

        if (!(left < right))
            break;
        std::iter_swap(left, right);
    }

    std::iter_swap(left, pivot);
    return left;
}

/// 在大区间使用三点取中快排，小区间留给插入排序完成收尾。
template<typename RandomIt, typename Compare>
void quickSortHybridImpl(RandomIt first, RandomIt last, Compare compare) {
    const typename std::iterator_traits<RandomIt>::difference_type cutoff = 16;

    while (last - first > cutoff) {
        RandomIt pivot = partitionMedianOfThree(first, last, compare);
        if (pivot - first < last - (pivot + 1)) {
            quickSortHybridImpl(first, pivot, compare);
            first = pivot + 1;
        } else {
            quickSortHybridImpl(pivot + 1, last, compare);
            last = pivot;
        }
    }
}

/// 在数组表示的堆中向下调整根节点，使区间重新满足 compare 对应的堆序。
template<typename RandomIt, typename Compare>
void siftDown(
    RandomIt first,
    typename std::iterator_traits<RandomIt>::difference_type root,
    typename std::iterator_traits<RandomIt>::difference_type count,
    Compare compare
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    for (;;) {
        Difference child = root * 2 + 1;
        if (child >= count)
            return;
        if (child + 1 < count && compare(*(first + child), *(first + child + 1)))
            ++child;
        if (!compare(*(first + root), *(first + child)))
            return;
        std::iter_swap(first + root, first + child);
        root = child;
    }
}

/// 为前向迭代器分派不依赖回退或随机访问的排序策略。
template<typename ForwardIt, typename Compare>
bool dispatchSort(
    ForwardIt first,
    ForwardIt last,
    SortStrategy strategy,
    Compare compare,
    std::forward_iterator_tag
);

/// 为双向迭代器增加插入排序能力，其余高阶策略由容器特化或随机访问实现承担。
template<typename BidirectionalIt, typename Compare>
bool dispatchSort(
    BidirectionalIt first,
    BidirectionalIt last,
    SortStrategy strategy,
    Compare compare,
    std::bidirectional_iterator_tag
);

/// 为随机访问迭代器分派完整的通用排序策略集合。
template<typename RandomIt, typename Compare>
bool dispatchSort(
    RandomIt first,
    RandomIt last,
    SortStrategy strategy,
    Compare compare,
    std::random_access_iterator_tag
);

} // namespace sort_detail

/// 使用相邻比较和交换执行稳定冒泡排序，仅要求可写前向迭代器。
template<typename ForwardIt, typename Compare>
void bubbleSort(ForwardIt first, ForwardIt last, Compare compare) {
    if (first == last)
        return;

    bool swapped;
    do {
        swapped = false;
        ForwardIt current = first;
        ForwardIt next = current;
        ++next;
        while (next != last) {
            if (compare(*next, *current)) {
                std::iter_swap(current, next);
                swapped = true;
            }
            ++current;
            ++next;
        }
    } while (swapped);
}

/// 使用 value_type 的默认小于关系执行冒泡排序。
template<typename ForwardIt>
void bubbleSort(ForwardIt first, ForwardIt last) {
    typedef typename std::iterator_traits<ForwardIt>::value_type Value;
    bubbleSort(first, last, std::less<Value>());
}

/// 每轮选择最优元素放到当前起点，支持可写前向迭代器。
template<typename ForwardIt, typename Compare>
void selectionSort(ForwardIt first, ForwardIt last, Compare compare) {
    for (ForwardIt current = first; current != last; ++current) {
        ForwardIt selected = current;
        ForwardIt candidate = current;
        ++candidate;
        while (candidate != last) {
            if (compare(*candidate, *selected))
                selected = candidate;
            ++candidate;
        }
        if (selected != current)
            std::iter_swap(current, selected);
    }
}

/// 使用 value_type 的默认小于关系执行选择排序。
template<typename ForwardIt>
void selectionSort(ForwardIt first, ForwardIt last) {
    typedef typename std::iterator_traits<ForwardIt>::value_type Value;
    selectionSort(first, last, std::less<Value>());
}

/// 通过向前交换把当前元素插入已排序前缀，要求可写双向迭代器。
template<typename BidirectionalIt, typename Compare>
void insertionSort(BidirectionalIt first, BidirectionalIt last, Compare compare) {
    if (first == last)
        return;

    BidirectionalIt current = first;
    ++current;
    while (current != last) {
        BidirectionalIt moving = current;
        while (moving != first) {
            BidirectionalIt previous = moving;
            --previous;
            if (!compare(*moving, *previous))
                break;
            std::iter_swap(moving, previous);
            moving = previous;
        }
        ++current;
    }
}

/// 使用 value_type 的默认小于关系执行插入排序。
template<typename BidirectionalIt>
void insertionSort(BidirectionalIt first, BidirectionalIt last) {
    typedef typename std::iterator_traits<BidirectionalIt>::value_type Value;
    insertionSort(first, last, std::less<Value>());
}

/// 使用 Knuth 间隔序列执行希尔排序，要求可写随机访问迭代器。
template<typename RandomIt, typename Compare>
void shellSort(RandomIt first, RandomIt last, Compare compare) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    const Difference count = last - first;
    Difference gap = 1;
    while (gap < count / 3)
        gap = 3 * gap + 1;

    while (gap >= 1) {
        for (Difference index = gap; index < count; ++index) {
            Difference moving = index;
            while (
                moving >= gap &&
                compare(*(first + moving), *(first + moving - gap))
            ) {
                std::iter_swap(first + moving, first + moving - gap);
                moving -= gap;
            }
        }
        gap /= 3;
    }
}

/// 使用 value_type 的默认小于关系执行希尔排序。
template<typename RandomIt>
void shellSort(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    shellSort(first, last, std::less<Value>());
}

/// 执行稳定的自顶向下归并排序，要求可写随机访问迭代器和可移动赋值元素。
template<typename RandomIt, typename Compare>
void mergeSort(RandomIt first, RandomIt last, Compare compare) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    const typename std::iterator_traits<RandomIt>::difference_type count =
        last - first;
    std::vector<Value> buffer;
    buffer.reserve(static_cast<std::size_t>(count));
    sort_detail::mergeSortTopDownImpl(first, last, compare, buffer);
}

/// 使用 value_type 的默认小于关系执行自顶向下归并排序。
template<typename RandomIt>
void mergeSort(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    mergeSort(first, last, std::less<Value>());
}

/// 执行稳定的自底向上归并排序，要求可写随机访问迭代器和可移动赋值元素。
template<typename RandomIt, typename Compare>
void mergeSortBottomUp(RandomIt first, RandomIt last, Compare compare) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;
    typedef typename std::iterator_traits<RandomIt>::value_type Value;

    const Difference count = last - first;
    std::vector<Value> buffer;
    buffer.reserve(static_cast<std::size_t>(count));
    for (Difference width = 1; width < count;) {
        for (Difference offset = 0; offset < count; offset += width * 2) {
            const Difference middleOffset =
                offset + width < count ? offset + width : count;
            const Difference endOffset =
                offset + width * 2 < count ? offset + width * 2 : count;
            if (
                middleOffset < endOffset &&
                compare(*(first + middleOffset), *(first + middleOffset - 1))
            ) {
                sort_detail::mergeRange(
                    first + offset,
                    first + middleOffset,
                    first + endOffset,
                    compare,
                    buffer
                );
            }
        }
        if (width > count / 2)
            break;
        width *= 2;
    }
}

/// 使用 value_type 的默认小于关系执行自底向上归并排序。
template<typename RandomIt>
void mergeSortBottomUp(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    mergeSortBottomUp(first, last, std::less<Value>());
}

/// 执行二路快速排序；固定枢轴实现不复制元素，但有序输入可能退化到 O(n²)。
template<typename RandomIt, typename Compare>
void quickSort(RandomIt first, RandomIt last, Compare compare) {
    sort_detail::quickSortImpl(first, last, compare);
}

/// 使用 value_type 的默认小于关系执行二路快速排序。
template<typename RandomIt>
void quickSort(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    quickSort(first, last, std::less<Value>());
}

/// 执行三向快速排序，重复元素较多时避免反复参与递归。
template<typename RandomIt, typename Compare>
void quick3WaySort(RandomIt first, RandomIt last, Compare compare) {
    sort_detail::quick3WaySortImpl(first, last, compare);
}

/// 使用 value_type 的默认小于关系执行三向快速排序。
template<typename RandomIt>
void quick3WaySort(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    quick3WaySort(first, last, std::less<Value>());
}

/// 执行三点取中与插入排序结合的混合快速排序。
template<typename RandomIt, typename Compare>
void quickSortHybrid(RandomIt first, RandomIt last, Compare compare) {
    sort_detail::quickSortHybridImpl(first, last, compare);
    insertionSort(first, last, compare);
}

/// 使用 value_type 的默认小于关系执行混合快速排序。
template<typename RandomIt>
void quickSortHybrid(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    quickSortHybrid(first, last, std::less<Value>());
}

/// 原地构造堆并逐步收缩堆区间，完成不稳定的堆排序。
template<typename RandomIt, typename Compare>
void heapSort(RandomIt first, RandomIt last, Compare compare) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    const Difference count = last - first;
    for (Difference start = count / 2; start > 0; --start)
        sort_detail::siftDown(first, start - 1, count, compare);

    for (Difference end = count; end > 1; --end) {
        std::iter_swap(first, first + end - 1);
        sort_detail::siftDown(first, 0, end - 1, compare);
    }
}

/// 使用 value_type 的默认小于关系执行堆排序。
template<typename RandomIt>
void heapSort(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::value_type Value;
    heapSort(first, last, std::less<Value>());
}

namespace sort_detail {

/// 为前向迭代器执行冒泡或选择排序，其他已知策略明确拒绝能力不足的区间。
template<typename ForwardIt, typename Compare>
bool dispatchSort(
    ForwardIt first,
    ForwardIt last,
    SortStrategy strategy,
    Compare compare,
    std::forward_iterator_tag
) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
        dsa::algorithm::bubbleSort(first, last, compare);
        return true;
    case SortStrategy::SelectionSort:
        dsa::algorithm::selectionSort(first, last, compare);
        return true;
    default:
        if (!isKnownStrategy(strategy))
            return false;
        throw std::invalid_argument(
            "The selected strategy requires bidirectional or random-access iterators"
        );
    }
}

/// 为双向迭代器执行冒泡、选择和插入排序，其余策略交给随机访问或容器特化。
template<typename BidirectionalIt, typename Compare>
bool dispatchSort(
    BidirectionalIt first,
    BidirectionalIt last,
    SortStrategy strategy,
    Compare compare,
    std::bidirectional_iterator_tag
) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
        dsa::algorithm::bubbleSort(first, last, compare);
        return true;
    case SortStrategy::SelectionSort:
        dsa::algorithm::selectionSort(first, last, compare);
        return true;
    case SortStrategy::InsertionSort:
        dsa::algorithm::insertionSort(first, last, compare);
        return true;
    default:
        if (!isKnownStrategy(strategy))
            return false;
        throw std::invalid_argument(
            "The selected strategy requires random-access iterators or a container specialization"
        );
    }
}

/// 为随机访问迭代器执行完整通用策略；基数排序仍保留为教学链表专用算法。
template<typename RandomIt, typename Compare>
bool dispatchSort(
    RandomIt first,
    RandomIt last,
    SortStrategy strategy,
    Compare compare,
    std::random_access_iterator_tag
) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
        dsa::algorithm::bubbleSort(first, last, compare);
        return true;
    case SortStrategy::SelectionSort:
        dsa::algorithm::selectionSort(first, last, compare);
        return true;
    case SortStrategy::InsertionSort:
        dsa::algorithm::insertionSort(first, last, compare);
        return true;
    case SortStrategy::ShellSort:
        dsa::algorithm::shellSort(first, last, compare);
        return true;
    case SortStrategy::MergeSort:
        dsa::algorithm::mergeSort(first, last, compare);
        return true;
    case SortStrategy::MergeSortB:
        dsa::algorithm::mergeSortBottomUp(first, last, compare);
        return true;
    case SortStrategy::QuickSort:
        dsa::algorithm::quickSort(first, last, compare);
        return true;
    case SortStrategy::Quick3way:
        dsa::algorithm::quick3WaySort(first, last, compare);
        return true;
    case SortStrategy::QuickSortB:
        dsa::algorithm::quickSortHybrid(first, last, compare);
        return true;
    case SortStrategy::HeapSort:
        dsa::algorithm::heapSort(first, last, compare);
        return true;
    case SortStrategy::RadixSort:
        throw std::invalid_argument(
            "RadixSort is only available for the teaching List"
        );
    }
    return false;
}

} // namespace sort_detail

/// 根据迭代器类别和策略执行排序；算法层不依赖任何仓库容器类型。
template<typename Iterator, typename Compare>
bool sort(
    Iterator first,
    Iterator last,
    SortStrategy strategy,
    Compare compare
) {
    typedef typename std::iterator_traits<Iterator>::iterator_category Category;
    return sort_detail::dispatchSort(first, last, strategy, compare, Category());
}

/// 使用 value_type 的默认小于关系执行指定策略排序。
template<typename Iterator>
bool sort(
    Iterator first,
    Iterator last,
    SortStrategy strategy = SortStrategy::QuickSort
) {
    typedef typename std::iterator_traits<Iterator>::value_type Value;
    return dsa::algorithm::sort(first, last, strategy, std::less<Value>());
}

} // namespace algorithm
} // namespace dsa

#endif

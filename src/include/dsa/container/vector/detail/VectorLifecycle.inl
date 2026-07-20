// -----------------------------------------------------------------------------
// Vector 构造、析构与赋值实现
// -----------------------------------------------------------------------------

/// 默认构造空 Vector。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector() noexcept(
    std::is_nothrow_default_constructible<allocator_type>::value
)
    : allocator_(),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
}

/// 使用指定 allocator 构造空 Vector。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(const allocator_type& allocator) noexcept
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
}

/// 构造 count 个值初始化元素。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    size_type count,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeDefault(count);
}

/// 构造 count 个 value 副本。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    size_type count,
    const value_type& value,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeFill(count, value);
}

/// 从迭代器区间构造 Vector，并按迭代器类别选择初始化策略。
template<typename T, typename Allocator>
template<typename InputIt>
Vector<T, Allocator>::Vector(
    InputIt first,
    InputIt last,
    const allocator_type& allocator,
    typename std::enable_if<!std::is_integral<InputIt>::value>::type*
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeRange(
        first,
        last,
        typename std::iterator_traits<InputIt>::iterator_category()
    );
}

/// 从 initializer_list 构造 Vector。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    std::initializer_list<value_type> values,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeRange(
        values.begin(),
        values.end(),
        std::forward_iterator_tag()
    );
}

/// 深拷贝构造，并遵循 allocator 的 copy-construction 选择规则。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(const Vector& other)
    : allocator_(
          allocator_traits::select_on_container_copy_construction(
              other.allocator_
          )
      ),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeRange(
        other.begin(),
        other.end(),
        std::forward_iterator_tag()
    );
}

/// 使用指定 allocator 深拷贝构造。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    const Vector& other,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeRange(
        other.begin(),
        other.end(),
        std::forward_iterator_tag()
    );
}

/// 移动构造并直接接管 other 的存储。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(Vector&& other) noexcept(
    std::is_nothrow_move_constructible<allocator_type>::value
)
    : allocator_(std::move(other.allocator_)),
      allocation_(other.allocation_),
      data_(other.data_),
      size_(other.size_),
      capacity_(other.capacity_) {
    other.resetStorage();
}

/// 使用指定 allocator 移动构造。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    Vector&& other,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    if (allocator_ == other.allocator_) {
        stealStorage(other);
        return;
    }

    initializeRange(
        std::make_move_iterator(other.begin()),
        std::make_move_iterator(other.end()),
        std::forward_iterator_tag()
    );
    other.clear();
}

/// 析构有效元素并释放底层存储。
template<typename T, typename Allocator>
Vector<T, Allocator>::~Vector() {
    clear();
    deallocateStorage();
}

/// 执行拷贝赋值并根据 allocator trait 选择传播策略。
template<typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(const Vector& other) {
    if (this == &other)
        return *this;

    copyAssign(
        other,
        typename allocator_traits::propagate_on_container_copy_assignment()
    );
    return *this;
}

/// 执行移动赋值并根据 allocator trait 选择传播策略。
template<typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(Vector&& other) noexcept(
    allocator_traits::propagate_on_container_move_assignment::value &&
    std::is_nothrow_move_assignable<allocator_type>::value
) {
    if (this == &other)
        return *this;

    moveAssign(
        other,
        typename allocator_traits::propagate_on_container_move_assignment()
    );
    return *this;
}

/// 使用 initializer_list 替换当前内容。
template<typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(
    std::initializer_list<value_type> values
) {
    assign(values.begin(), values.end());
    return *this;
}

/// 将内容替换为 count 个 value 副本。
template<typename T, typename Allocator>
void Vector<T, Allocator>::assign(
    size_type count,
    const value_type& value
) {
    Vector replacement(count, value, allocator_);
    swapStorage(replacement);
}

/// 将内容替换为迭代器区间元素。
template<typename T, typename Allocator>
template<typename InputIt>
typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
Vector<T, Allocator>::assign(InputIt first, InputIt last) {
    Vector replacement(first, last, allocator_);
    swapStorage(replacement);
}

/// 将内容替换为 initializer_list 元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::assign(
    std::initializer_list<value_type> values
) {
    assign(values.begin(), values.end());
}

/// 返回 allocator 副本。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::allocator_type
Vector<T, Allocator>::get_allocator() const noexcept {
    return allocator_;
}

// -----------------------------------------------------------------------------
// 元素访问与迭代器实现
// -----------------------------------------------------------------------------

/// 返回带越界检查的可写元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::at(size_type position) {
    checkPosition(position);
    return data_[position];
}

/// 返回带越界检查的只读元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::at(size_type position) const {
    checkPosition(position);
    return data_[position];
}

/// 返回不做边界检查的可写元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::operator[](size_type position) noexcept {
    return data_[position];
}

/// 返回不做边界检查的只读元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::operator[](size_type position) const noexcept {
    return data_[position];
}

/// 返回首元素可写引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::front() noexcept {
    return data_[0];
}

/// 返回首元素只读引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::front() const noexcept {
    return data_[0];
}

/// 返回尾元素可写引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::back() noexcept {
    return data_[size_ - 1];
}

/// 返回尾元素只读引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::back() const noexcept {
    return data_[size_ - 1];
}

/// 返回连续存储首地址。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::pointer
Vector<T, Allocator>::data() noexcept {
    return data_;
}

/// 返回连续存储首地址的只读指针。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_pointer
Vector<T, Allocator>::data() const noexcept {
    return data_;
}

/// 返回首元素迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::begin() noexcept {
    return data_;
}

/// 返回首元素只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::begin() const noexcept {
    return data_;
}

/// 返回首元素只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::cbegin() const noexcept {
    return data_;
}

/// 返回尾后迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::end() noexcept {
    return size_ == 0 ? data_ : data_ + size_;
}

/// 返回尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::end() const noexcept {
    return size_ == 0 ? data_ : data_ + size_;
}

/// 返回尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::cend() const noexcept {
    return size_ == 0 ? data_ : data_ + size_;
}

/// 返回反向首迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reverse_iterator
Vector<T, Allocator>::rbegin() noexcept {
    return reverse_iterator(end());
}

/// 返回反向首只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::rbegin() const noexcept {
    return const_reverse_iterator(end());
}

/// 返回反向首只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::crbegin() const noexcept {
    return const_reverse_iterator(cend());
}

/// 返回反向尾后迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reverse_iterator
Vector<T, Allocator>::rend() noexcept {
    return reverse_iterator(begin());
}

/// 返回反向尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::rend() const noexcept {
    return const_reverse_iterator(begin());
}

/// 返回反向尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::crend() const noexcept {
    return const_reverse_iterator(cbegin());
}

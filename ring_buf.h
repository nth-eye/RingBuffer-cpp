#include <cstddef>


// Undef to overwrite old values when buffer is full.
#define RING_BUF_NODISCARD

template<size_t N>
// Greater or equal to 2 constraint.
using ge_2 = std::enable_if_t<N >= 2>*;

template<size_t N>
// Power of 2 constraint.
using pow_2 = std::enable_if_t<(N & (N - 1)) == 0>*;


// SECTION Iterators

// Generic iterator for ring buffer classes.
// \tparam T Value type which buffer holds.
template<class T>
class ring_iterator {
protected:
    T		*ring_buf;
    size_t	pos;
public:
    ring_iterator(T *ring_buf_, size_t pos_) : ring_buf(ring_buf_), pos(pos_) {}

    T& operator*()  { return ring_buf[pos]; }
    T* operator->() { return &(operator*()); }
    ring_iterator& operator++()     { ++pos; return *this; }
    ring_iterator operator++(int)   { T tmp(*this); operator++(); return tmp; }

    bool operator==(const ring_iterator &other) { return pos == other.pos; }
    bool operator!=(const ring_iterator &other) { return !operator==(other); }
};

// Iterator for ring buffer with "modulo" implementation.
template<class T, size_t N>
class ring_iterator_mod : public ring_iterator<T> {
public:
    ring_iterator_mod(T *ring_buf_, size_t pos_) : ring_iterator<T>(ring_buf_, pos_) {}
    ring_iterator_mod& operator++() { if (++this->pos == N) this->pos = 0; return *this; }
};

// Iterator for ring buffer with cyclic "power of 2" implementation.
// Applies the mask at the moment of increment.
template<class T, size_t Mask>
class ring_iterator_pow_inc : public ring_iterator<T> {
public:
    ring_iterator_pow_inc(T *ring_buf_, size_t pos_) : ring_iterator<T>(ring_buf_, pos_) {}
    ring_iterator_pow_inc& operator++() { ++this->pos &= Mask; return *this; }
};

// Iterator for ring buffer with cyclic "power of 2" implementation.
// Applies the mask at the moment of dereference.
template<class T, size_t Mask>
class ring_iterator_pow_ref : public ring_iterator<T> {
public:
    ring_iterator_pow_ref(T *ring_buf_, size_t pos_) : ring_iterator<T>(ring_buf_, pos_) {}
    T& operator*()  { return this->ring_buf[this->pos & Mask]; }
};

// !SECTION Iterators


// SECTION Ring buffers

// Circular buffer base implementation.
// \tparam T Data type to be stored.
// \tparam N Max buffer size.
// \tparam iterator Custom iterator for ring buffer type.
template<class T, size_t N, class iterator>
class ring_buf_base {
public:
    T buf[N] = {};
    size_t head = 0;    // First item index / beginning of the buffer.
    size_t tail = 0;    // Last item index.
public:
    iterator begin()            { return iterator(buf, head); }
    iterator end()              { return iterator(buf, tail); }
    T& operator[](size_t idx)   { return buf[idx]; }
    T& front()                  { return operator[](head); }
    T& back()                   { return operator[](tail); }
    const iterator begin() const            { return iterator(buf, head); }
    const iterator end() const              { return iterator(buf, tail); }
    const T& operator[](size_t idx) const   { return buf[idx]; }
    const T& front() const					{ return operator[](head); }
    const T& back() const					{ return operator[](tail); }
};

// Uses modulo and N-1 elements logic, so it can have arbitrary storage of N-1 size.
// \tparam T Data type to be stored.
// \tparam N Max buffer size. Must be >= 2.
template<class T, size_t N, ge_2<N> = nullptr>
class ring_buf_ver_1 : public ring_buf_base<T, N, ring_iterator_mod<T, N>> {
    using ring_buf_ver_1::ring_buf_base::head;
    using ring_buf_ver_1::ring_buf_base::tail;
    using ring_buf_ver_1::ring_buf_base::buf;
public:
    void push_back(const T &item)
    {
        size_t next = tail + 1;
        if (next == N) next = 0;
        if (next == head)
#ifdef RING_BUF_NODISCARD
            return;
#else
            pop_front();
#endif
        buf[tail] = item;
        tail = next;
    }
    // Responsibility to check empty() is on the caller.
    void pop_front()                    { if (++head == N) head = 0; }
    void clear()                        { head = tail = 0; }
    bool empty() const                  { return tail == head; }
    bool full() const                   { return ((tail + 1) % N) == head; }
    size_t size() const                 { return tail > head ? tail - head: N + tail - head; }
    size_t constexpr capacity() const   { return N - 1; }
};

// Uses pow_2 and N-1 elements logic. Provides storage capacity of N-1.
// \tparam T Data type to be stored.
// \tparam N Max buffer size. Must be power of 2 and >= 2.
template<class T, size_t N, ge_2<N> = nullptr, pow_2<N> = nullptr>
class ring_buf_ver_2 : public ring_buf_base<T, N, ring_iterator_pow_inc<T, N - 1>> {
    using ring_buf_ver_2::ring_buf_base::head;
    using ring_buf_ver_2::ring_buf_base::tail;
    using ring_buf_ver_2::ring_buf_base::buf;
    static constexpr auto mask = N - 1;
public:
    void push_back(const T &item)
    {
        size_t next = (tail + 1) & mask;
        if (next == head)
#ifdef RING_BUF_NODISCARD
            return;
#else
            pop_front();
#endif
        buf[tail] = item;
        tail = next;
    }
    // Responsibility to check empty() is on the caller.
    void pop_front()                    { ++head &= mask; }
    void clear()                        { head = tail = 0; }
    bool empty() const                  { return tail == head; }
    bool full() const                   { return ((tail + 1) & mask) == head; }
    size_t size() const                 { return tail > head ? tail - head: N + tail - head; }
    size_t constexpr capacity() const   { return N - 1; }
};

// Uses pow_2 and (read + length) indexing logic. Provides storage capacity of N.
// \tparam T Data type to be stored.
// \tparam N Max buffer size. Must be power of 2.
template<class T, size_t N, pow_2<N> = nullptr>
class ring_buf_ver_3 : public ring_buf_base<T, N, ring_iterator_pow_ref<T, N - 1>> {
public:
    using ring_buf_ver_3::ring_buf_base::head;  // Consider as read index here.
    using ring_buf_ver_3::ring_buf_base::tail;  // Consider as length.
    using ring_buf_ver_3::ring_buf_base::buf;
    using iterator = ring_iterator_pow_ref<T, N - 1>;
    static constexpr auto mask = N - 1;
public:
    iterator end()      { return iterator(buf, head + tail); }
    T& back()           { return operator[](head + tail); }
    const iterator end() const      { return iterator(buf, head + tail); }
    const T& back() const           { return operator[](head + tail); }

    void push_back(const T &item)	
    {
        if (full()) 
#ifdef RING_BUF_NODISCARD
            return; 
#else
            pop_front();
#endif 
        buf[(head + tail++) & mask] = item;
    }
    // Responsibility to check empty() is on the caller.
    void pop_front()                    { ++head &= mask; --tail; }
    void clear()                        { head = tail = 0; }
    bool empty() const                  { return tail == 0; }
    bool full() const                   { return tail == N; }
    size_t size() const                 { return tail; }
    size_t constexpr capacity() const   { return N; }
};

// Uses pow_2 and unmasked indices logic. Provides storage capacity of N
// \tparam T Data type to be stored.
// \tparam N Max buffer size. Must be power of 2.
template<class T, size_t N, pow_2<N> = nullptr>
class ring_buf_ver_4 : public ring_buf_base<T, N, ring_iterator_pow_ref<T, N - 1>> {
    using ring_buf_ver_4::ring_buf_base::head;
    using ring_buf_ver_4::ring_buf_base::tail;
    using ring_buf_ver_4::ring_buf_base::buf;
    static constexpr auto mask = N - 1;
public:
    void push_back(const T &item)	
    {
        if (full()) 
#ifdef RING_BUF_NODISCARD
            return; 
#else
            pop_front();
#endif 
        buf[tail++ & mask] = item;
    }
    // Responsibility to check empty() is on the caller.
    void pop_front()                    { ++head; }
    void clear()                        { head = tail = 0; }
    bool empty() const                  { return tail == head; }
    bool full() const                   { return size() == N; }
    size_t size() const                 { return tail - head; }
    size_t constexpr capacity() const   { return N; }
};

// !SECTION Ring buffers


template<class T, size_t N>
// Name alias for ring buffer implementation which you want to use.
// \tparam T Value type.
// \tparam N Size.
using ring_buf = ring_buf_ver_4<T, N>; 

#pragma once
#include <iterator>

template<typename T>
class FTArrayIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    FTArrayIterator() = default;
    FTArrayIterator(T* ptr) : m_ptr(ptr) {}

    FTArrayIterator& operator++() { ++m_ptr; return *this; }
    FTArrayIterator operator++(int) { FTArrayIterator tmp = *this; ++m_ptr; return tmp; }
    FTArrayIterator& operator--() { --m_ptr; return *this; }
    FTArrayIterator operator--(int) { FTArrayIterator tmp = *this; --m_ptr; return tmp; }

    reference operator*() { return *m_ptr; }
    pointer operator->() { return m_ptr; }

    bool operator==(const FTArrayIterator& rhs) const { return m_ptr == rhs.m_ptr; }
    bool operator!=(const FTArrayIterator& rhs) const { return m_ptr != rhs.m_ptr; }

    FTArrayIterator operator+(difference_type n) const { return FTArrayIterator(m_ptr + n); }
    FTArrayIterator operator-(difference_type n) const { return FTArrayIterator(m_ptr - n); }
    difference_type operator-(const FTArrayIterator& rhs) const { return m_ptr - rhs.m_ptr; }

    FTArrayIterator& operator+=(difference_type n) { m_ptr += n; return *this; }
    FTArrayIterator& operator-=(difference_type n) { m_ptr -= n; return *this; }

    reference operator[](difference_type n) const { return m_ptr[n]; }

private:
    T* m_ptr;
};
#pragma once
#include <iterator>

template<typename T>
class FTArrayIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    explicit FTArrayIterator(T* pPtr) : m_pPtr(pPtr) {}
    FTArrayIterator() : m_pPtr(nullptr) {}

    FTArrayIterator& operator++() { ++m_pPtr; return *this; }
    FTArrayIterator operator++(int) { FTArrayIterator tmp = *this; ++m_pPtr; return tmp; }
    FTArrayIterator& operator--() { --m_pPtr; return *this; }
    FTArrayIterator operator--(int) { FTArrayIterator tmp = *this; --m_pPtr; return tmp; }

    reference operator*() { return *m_pPtr; }
    pointer operator->() { return m_pPtr; }

    bool operator==(const FTArrayIterator& rhs) const { return m_pPtr == rhs.m_pPtr; }
    bool operator!=(const FTArrayIterator& rhs) const { return m_pPtr != rhs.m_pPtr; }

    FTArrayIterator operator+(difference_type n) const { return FTArrayIterator(m_pPtr + n); }
    FTArrayIterator operator-(difference_type n) const { return FTArrayIterator(m_pPtr - n); }
    difference_type operator-(const FTArrayIterator& rhs) const { return m_pPtr - rhs.m_pPtr; }

    FTArrayIterator& operator+=(difference_type n) { m_pPtr += n; return *this; }
    FTArrayIterator& operator-=(difference_type n) { m_pPtr -= n; return *this; }

    reference operator[](difference_type n) const { return m_pPtr[n]; }

private:
    T* m_pPtr;
};
#pragma once
#include <iterator>

template<typename T>
class FTArrayIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = size_t;
    using pointer = T*;
    using reference = T&;

    constexpr explicit FTArrayIterator(T* pPtr) noexcept : m_pPtr(pPtr) {}
    constexpr FTArrayIterator() noexcept : m_pPtr(nullptr) {}

    constexpr FTArrayIterator& operator++() noexcept { ++m_pPtr; return *this; }
    constexpr FTArrayIterator operator++(int) noexcept { FTArrayIterator tmp = *this; ++m_pPtr; return tmp; }
    constexpr FTArrayIterator& operator++() const noexcept { ++m_pPtr; return *this; }
    constexpr FTArrayIterator operator++(int) const noexcept { FTArrayIterator tmp = *this; ++m_pPtr; return tmp; }

    constexpr FTArrayIterator& operator--() noexcept { --m_pPtr; return *this; }
    constexpr FTArrayIterator operator--(int) { FTArrayIterator tmp = *this; --m_pPtr; return tmp; }
    constexpr FTArrayIterator& operator--() const noexcept { --m_pPtr; return *this; }
    constexpr FTArrayIterator operator--(int) const noexcept { FTArrayIterator tmp = *this; --m_pPtr; return tmp; }

    constexpr reference operator*() noexcept { return *m_pPtr; }
    constexpr pointer operator->() noexcept { return m_pPtr; }
     constexpr reference operator*() const noexcept { return *m_pPtr; }
    constexpr pointer operator->() const noexcept { return m_pPtr; }

    constexpr bool operator==(const FTArrayIterator& rhs) const noexcept { return m_pPtr == rhs.m_pPtr; }
    constexpr bool operator!=(const FTArrayIterator& rhs) const noexcept { return m_pPtr != rhs.m_pPtr; }

    constexpr FTArrayIterator operator+(difference_type n) const noexcept { return FTArrayIterator(m_pPtr + n); }
    constexpr FTArrayIterator operator-(difference_type n) const noexcept { return FTArrayIterator(m_pPtr - n); }
    constexpr difference_type operator-(const FTArrayIterator& rhs) const noexcept { return m_pPtr - rhs.m_pPtr; }

    constexpr FTArrayIterator& operator+=(difference_type n) noexcept { m_pPtr += n; return *this; }
    constexpr FTArrayIterator& operator-=(difference_type n) noexcept { m_pPtr -= n; return *this; }
	constexpr FTArrayIterator& operator+=(difference_type n) const noexcept { m_pPtr += n; return *this; }
    constexpr FTArrayIterator& operator-=(difference_type n) const noexcept { m_pPtr -= n; return *this; }

    constexpr reference operator[](difference_type n) const noexcept { return m_pPtr[n]; }

private:
    T* m_pPtr;
};
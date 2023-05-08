#pragma once
#include <Windows.h>

#include "Globals.h"

template<typename T>
class FTMemory
{
public:

	__forceinline explicit FTMemory(const int nGrowSize = 0, const int nInitialAllocationCount = 0) noexcept
		: m_pMemory(nullptr), m_nGrowSize(nGrowSize), m_nAllocationCount(nInitialAllocationCount)
	{
		FT_ASSERT(nGrowSize >= 0);

		if (m_nAllocationCount)
			m_pMemory = static_cast<T*>(_aligned_malloc(static_cast<size_t>(m_nAllocationCount) 
				* sizeof(T), sizeof(T)));
	}

	class Iterator
	{
	public:
		__forceinline explicit Iterator(const int i) : m_nIndex(i) {}

		__forceinline bool operator==(const Iterator& it) const noexcept
		{
			return m_nIndex == it.m_nIndex;
		}

		__forceinline bool operator!=(const Iterator& it) const noexcept
		{
			return m_nIndex != it.m_nIndex;
		}

		int m_nIndex;
	};

	__forceinline Iterator First(Iterator it) const noexcept
	{
		return Iterator(IsIndexValid(0) ? 0 : FT_INVALID_INDEX);
	}

	__forceinline Iterator Next(Iterator it) const noexcept
	{
		return Iterator(IsIndexValid(it.index + 1) ? it.index + 1 : FT_INVALID_INDEX);
	}

	__forceinline bool IsIndexValid(const unsigned int nIndex) const noexcept
	{
		return (nIndex < m_nAllocationCount);
	}

	__forceinline bool IsIndexValid(const int nIndex) const noexcept
	{
		return (nIndex >= 0) && (nIndex < m_nAllocationCount);
	}

	__forceinline T& At(int nIndex) noexcept
	{
		FT_ASSERT(IsIndexValid(nIndex));
		return m_pMemory[nIndex];
	}

	__forceinline const T& At(int nIndex) const noexcept
	{
		FT_ASSERT(IsIndexValid(nIndex));
		return m_pMemory[nIndex];
	}

	__forceinline T& operator[](const int nIndex) noexcept
	{
		return At(nIndex);
	}

	__forceinline const T& operator[](const int nIndex) const noexcept
	{
		return At(nIndex);
	}

	__forceinline bool IsExternallyAllocated() const noexcept
	{
		return (m_nGrowSize < 0);
	}

	__forceinline void SetGrowSize(const int nSize) noexcept
	{
		FT_ASSERT(!IsExternallyAllocated());
		FT_ASSERT(nSize >= 0);
		m_nGrowSize = nSize;
	}

	__forceinline T* Base() noexcept
	{
		return m_pMemory;
	}

	__forceinline const T* Base() const noexcept
	{
		return m_pMemory;
	}

	__forceinline int GetAllocationCount() const noexcept
	{
		return m_nAllocationCount;
	}

	__forceinline int CalcNewAllocationCount(int nAllocationCount, const int nGrowSize,
		const int nNewSize, const int nBytesItem) const noexcept
	{
		// https://github.com/Jasper1467/BitManipulation/blob/master/BitManipulation/include/BitManipulation.h#L11

		if (nGrowSize)
		{
			// If nGrowSize is a power of 2, we can use bitwise instead of division
			if (m_bGrowSizeIsPowerOf2)
				nAllocationCount = ((1 + ((nNewSize & ~(nNewSize - 1)) / nGrowSize)) * nGrowSize);
			else
				nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
		}
		else
		{
			if (!nAllocationCount)
			{
				if (m_bGrowSizeIsPowerOf2)
				{
					nAllocationCount = (FT_ALLOC_SIZE_PRIME + nBytesItem) >> 5;

					// Add 1 if nBytesItem * nAllocationCount < FT_ALLOC_SIZE_PRIME
					nAllocationCount += ((nBytesItem * nAllocationCount) < FT_ALLOC_SIZE_PRIME);
				}
				else
					nAllocationCount = (FT_ALLOC_SIZE_PRIME + nBytesItem) / nBytesItem;

				if (nAllocationCount < nNewSize)
					nAllocationCount = nNewSize;
			}

			while (nAllocationCount < nNewSize)
			{

				const int nNewAllocationCount = (nAllocationCount >> 3)
					+ (nAllocationCount >> 4) + nAllocationCount; // 1/8 + 1/16 + 1 = 1.3125

				nAllocationCount = (nNewAllocationCount < nAllocationCount)
					? (nAllocationCount << 1) : nNewAllocationCount;
			}
		}

		return nAllocationCount;
	}

	__forceinline void Grow(const int nNum, const bool bUsePowerOfTwoGrowth = true) noexcept
	{
		FT_ASSERT(nNum > 0);

		if (IsExternallyAllocated())
		{
			FT_ASSERT(0); // Can't grow a buffer whose memory was externally allocated
			return;
		}

		m_bGrowSizeIsPowerOf2 = m_nGrowSize && (!(m_nGrowSize & (m_nGrowSize - 1)));

		const int nAllocationRequested = m_nAllocationCount + nNum;
		int nNewAllocationCount = CalcNewAllocationCount(m_nAllocationCount, m_nGrowSize,
			nAllocationRequested, sizeof(T));

		if (nNewAllocationCount < nAllocationRequested)
		{
			if (!nNewAllocationCount && (nNewAllocationCount - 1) >= nAllocationRequested)
				--nNewAllocationCount;
			else
			{
				while (nNewAllocationCount < nAllocationRequested)
				{
					if (bUsePowerOfTwoGrowth)
						// Use power of two growth strategy
						nNewAllocationCount = nNewAllocationCount << 1; // Equivalent of *= 2
					else
						nNewAllocationCount = (nNewAllocationCount + nAllocationRequested) / 2;
				}
			}
		}

		m_nAllocationCount = nNewAllocationCount;

		T* pNewMemory;
		if (m_pMemory)
			pNewMemory = static_cast<T*>(_aligned_realloc(m_pMemory, static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T)));
		else
			pNewMemory = static_cast<T*>(_aligned_malloc(static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T)));

		FT_ASSERT(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

	__forceinline void EnsureCapacity(const int nNum) noexcept
	{
		if (m_nAllocationCount >= nNum)
			return;

		if (IsExternallyAllocated())
		{
			// Can't grow a buffer whose memory was externally allocated 
			FT_ASSERT(0);
			return;
		}

		m_nAllocationCount = nNum;

		T* pNewMemory;
		if (m_pMemory)
			pNewMemory = static_cast<T*>(_aligned_realloc(m_pMemory, static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T)));
		else
			pNewMemory = static_cast<T*>(_aligned_malloc(static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T)));

		FT_ASSERT(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

	__forceinline void Purge() noexcept
	{
		if (!IsExternallyAllocated())
		{
			if (m_pMemory)
			{
				_aligned_free(m_pMemory);
				m_pMemory = nullptr;
			}

			m_nAllocationCount = 0;
		}
	}

	__forceinline void Purge(const int nCount) noexcept
	{
		FT_ASSERT(nCount >= 0);

		if (nCount > m_nAllocationCount)
		{
			// Ensure this isn't a grow request in disguise.
			FT_ASSERT(nCount <= m_nAllocationCount);
			return;
		}

		// If we have zero elements, simply do a purge:
		if (nCount == 0)
		{
			Purge();
			return;
		}

		if (IsExternallyAllocated())
		{
			// Can't shrink a buffer whose memory was externally allocated, fail silently like purge 
			return;
		}

		// If the number of elements is the same as the allocation count, we are done.
		if (nCount == m_nAllocationCount)
			return;

		if (!m_pMemory)
		{
			// Allocation count is non zero, but memory is null.
			assert(m_pMemory != nullptr);
			return;
		}

		m_nAllocationCount = nCount;

		T* pNewMemory = static_cast<T*>(_aligned_realloc(m_pMemory, static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T)));

		FT_ASSERT(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

private:
	T* m_pMemory = nullptr;
	int m_nGrowSize = 0;
	int m_nAllocationCount = 0;
	bool m_bGrowSizeIsPowerOf2 = false;
};

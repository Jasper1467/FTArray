#pragma once
#include <Windows.h>

#include "Globals.h"

template<typename T>
class FTMemory
{
public:

	explicit FTMemory(const int nGrowSize = 0, const int nInitialAllocationCount = 0)
		: m_pMemory(nullptr), m_nGrowSize(nGrowSize), m_nAllocationCount(nInitialAllocationCount)
	{
		FT_ASSERT(nGrowSize >= 0);

		if (m_nAllocationCount)
			m_pMemory = (T*)_aligned_malloc(static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T));
	}

	class Iterator
	{
	public:
		explicit Iterator(int i) : m_nIndex(i) {}

		bool operator==(const Iterator& it) const
		{
			return m_nIndex == it.m_nIndex;
		}

		bool operator!=(const Iterator& it) const
		{
			return m_nIndex != it.m_nIndex;
		}

		int m_nIndex;
	};

	Iterator First(Iterator it) const
	{
		return Iterator(IsIndexValid(0) ? 0 : FT_INVALID_INDEX);
	}

	Iterator Next(Iterator it) const
	{
		return Iterator(IsIndexValid(it.index + 1) ? it.index + 1 : FT_INVALID_INDEX);
	}

	bool IsIndexValid(const int nIndex) const
	{
		return (nIndex >= 0) && (nIndex < m_nAllocationCount);
	}

	T& At(int nIndex)
	{
		FT_ASSERT(IsIndexValid(nIndex));
		return m_pMemory[nIndex];
	}

	const T& At(int nIndex) const
	{
		FT_ASSERT(IsIndexValid(nIndex));
		return m_pMemory[nIndex];
	}

	T& operator[](const int nIndex)
	{
		return At(nIndex);
	}

	const T& operator[](const int nIndex) const
	{
		return At(nIndex);
	}

	bool IsExternallyAllocated() const
	{
		return (m_nGrowSize < 0);
	}

	void SetGrowSize(const int nSize)
	{
		FT_ASSERT(!IsExternallyAllocated());
		FT_ASSERT(nSize >= 0);
		m_nGrowSize = nSize;
	}

	T* Base()
	{
		return m_pMemory;
	}

	const T* Base() const
	{
		return m_pMemory;
	}

	int GetAllocationCount() const
	{
		return m_nAllocationCount;
	}

	int CalcNewAllocationCount(int nAllocationCount, const int nGrowSize,
		const int nNewSize, const int nBytesItem) const
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

	void Grow(const int nNum, const bool bUsePowerOfTwoGrowth = true)
	{
		FT_ASSERT(nNum > 0);

		if (IsExternallyAllocated())
		{
			FT_ASSERT(0); // Can't grow a buffer whose memory was externally allocated
			return;
		}

		m_bGrowSizeIsPowerOf2 = m_nGrowSize && (!m_nGrowSize & (m_nGrowSize - 1));

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
			pNewMemory = (T*)_aligned_realloc(m_pMemory, static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T));
		else
			pNewMemory = (T*)_aligned_malloc(static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T));

		FT_ASSERT(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

	void EnsureCapacity(const int nNum)
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
			pNewMemory = (T*)_aligned_realloc(m_pMemory, static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T));
		else
			pNewMemory = (T*)_aligned_malloc(static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T));

		FT_ASSERT(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

	void Purge()
	{
		if (!IsExternallyAllocated())
		{
			if (m_pMemory)
			{
				free(m_pMemory);
				m_pMemory = nullptr;
			}

			m_nAllocationCount = 0;
		}
	}

	void Purge(const int nCount)
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

		T* pNewMemory = (T*)_aligned_realloc(m_pMemory, static_cast<size_t>(m_nAllocationCount) * sizeof(T), sizeof(T));

		FT_ASSERT(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

private:
	T* m_pMemory = nullptr;
	int m_nGrowSize = 0;
	int m_nAllocationCount = 0;
	bool m_bGrowSizeIsPowerOf2 = false;
};

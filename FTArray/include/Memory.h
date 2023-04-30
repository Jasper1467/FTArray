#pragma once
#include <cassert>
#include <Windows.h>

#include "Globals.h"

template<typename T>
class FTMemory
{
public:

	explicit FTMemory(int nGrowSize = 0, int nInitialAllocationCount = 0)
		: m_pMemory(nullptr), m_nGrowSize(nGrowSize), m_nAllocationCount(nInitialAllocationCount)
	{
		assert(nGrowSize >= 0);

		if (m_nAllocationCount)
			m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
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
		return Iterator(IsIndexValid(0) ? 0 : details::INVALID_INDEX);
	}

	Iterator Next(Iterator it) const
	{
		return Iterator(IsIndexValid(it.index + 1) ? it.index + 1 : details::INVALID_INDEX);
	}

	bool IsIndexValid(const int nIndex) const
	{
		return (nIndex >= 0) && (nIndex < m_nAllocationCount);
	}

	T& At(int nIndex)
	{
		assert(IsIndexValid(nIndex));
		return m_pMemory[nIndex];
	}

	const T& At(int nIndex) const
	{
		assert(IsIndexValid(nIndex));
		return m_pMemory[nIndex];
	}

	T& operator[](const int nIndex)
	{
		return At(nIndex);
	}

	const T& operator[](int nIndex) const
	{
		return At(nIndex);
	}

	bool IsExternallyAllocated() const
	{
		return (m_nGrowSize < 0);
	}

	void SetGrowSize(const int nSize)
	{
		assert(!IsExternallyAllocated());
		assert(nSize >= 0);
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

	static int CalcNewAllocationCount(int nAllocationCount, const int nGrowSize,
		const int nNewSize, const int nBytesItem)
	{
		if (nGrowSize)
			nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
		else
		{
			if (!nAllocationCount)
			{
				nAllocationCount = (31 + nBytesItem) / nBytesItem;

				if (nAllocationCount < nNewSize)
					nAllocationCount = nNewSize;
			}

			while (nAllocationCount < nNewSize)
			{
				const int nNewAllocationCount = (nAllocationCount * 9) / 8; // 12.5%
				if (nNewAllocationCount > nAllocationCount)
					nAllocationCount = nNewAllocationCount;
				else
					nAllocationCount *= 2;
			}
		}

		return nAllocationCount;
	}

	void Grow(const int nNum)
	{
		assert(nNum > 0);

		if (IsExternallyAllocated())
		{
			assert(0); // Can't grow a buffer whose memory was externally allocated
			return;
		}

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
					nNewAllocationCount = (nNewAllocationCount + nAllocationRequested) / 2;
			}
		}

		m_nAllocationCount = nNewAllocationCount;

		T* pNewMemory;
		if (m_pMemory)
			pNewMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
		else
			pNewMemory = (T*)malloc(m_nAllocationCount * sizeof(T));

		assert(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

	void EnsureCapacity(const int nNum)
	{
		if (m_nAllocationCount >= nNum)
			return;

		if (IsExternallyAllocated())
		{
			// Can't grow a buffer whose memory was externally allocated 
			assert(0);
			return;
		}

		m_nAllocationCount = nNum;

		T* pNewMemory;
		if (m_pMemory)
			pNewMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
		else
			pNewMemory = (T*)malloc(m_nAllocationCount * sizeof(T));

		assert(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

	void Purge()
	{
		if (!IsExternallyAllocated())
		{
			if (m_pMemory)
			{
				free((void*)m_pMemory);
				m_pMemory = nullptr;
			}

			m_nAllocationCount = 0;
		}
	}

	void Purge(const int nCount)
	{
		assert(nCount >= 0);

		if (nCount > m_nAllocationCount)
		{
			// Ensure this isn't a grow request in disguise.
			assert(nCount <= m_nAllocationCount);
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

		T* pNewMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));

		assert(pNewMemory == nullptr);

		m_pMemory = pNewMemory;
	}

private:
	T* m_pMemory = nullptr;
	int m_nGrowSize = 0;
	int m_nAllocationCount = 0;
};

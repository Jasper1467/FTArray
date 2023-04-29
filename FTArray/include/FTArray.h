#pragma once
#include <cstdlib>

class FTMemoryPool
{
public:
	FTMemoryPool(const size_t nSize, const size_t nBlockSize)
	{
		m_nSize = nSize;
		m_pPtr = Allocate(nSize);
	}

	~FTMemoryPool()
	{
		Deallocate(m_pPtr, m_nSize);
		Destroy();
	}

	void* Allocate(const size_t nSize) const
	{
		const size_t nIndex = nSize / sizeof(void*);
		if (m_ppFreeList[nIndex] != nullptr)
		{
			void* pPtr = m_ppFreeList[nIndex];
			m_ppFreeList[nIndex] = *(void**)pPtr;
			return pPtr;
		}

		return nullptr;
	}

	void Deallocate(void* pPtr, const size_t nSize) const
	{
		*(void**)pPtr = m_ppFreeList[nSize / sizeof(void*)];
		m_ppFreeList[nSize / sizeof(void*)] = pPtr;
	}

	void Destroy()
	{
		free(m_ppFreeList);
		free(m_pBuffer);
		free(this);
	}

	size_t m_nSize = 0;
	char* m_pBuffer = nullptr;
	void** m_ppFreeList = nullptr;
	void* m_pPtr = nullptr;
};

// T = var type, N = amount of items in array
template<typename T, size_t N = 0>
class FTArray
{
	class Iterator
	{
	public:
		explicit Iterator(T* pPtr)
		{
			m_pPtr = pPtr;
		}

		Iterator& operator++()
		{
			++m_pPtr;
			return *this;
		}

	private:
		T* m_pPtr = nullptr;
	};

	static void CreatePool(const size_t nSize, const size_t nBlockSize)
	{
		FTMemoryPool* pPool = (FTMemoryPool*)malloc(sizeof(FTMemoryPool));
		pPool->m_nSize = nSize;
		pPool->m_pBuffer = (char*)malloc(nSize);
		*pPool->m_ppFreeList = malloc((nSize / nBlockSize) * sizeof(void*));

		for (size_t i = 0; i < nSize / nBlockSize; i++)
			pPool->m_ppFreeList[i] = (void*)(pPool->m_pBuffer + i * nBlockSize);
	}

	FTMemoryPool* m_pPool = nullptr;
	T* m_pArray = nullptr;

public:
	explicit FTArray()
	{
		CreatePool(N * sizeof(T), sizeof(T));
		m_pArray = (T*)m_pPool->m_pPtr;
	}

	T operator[](int nIndex)
	{
		return m_pArray[nIndex];
	}

	void ExtendSize(const size_t nAmount)
	{
		m_nWishPoolSize += nAmount;

		m_pPool->~FTMemoryPool();
		CreatePool(m_nWishPoolSize, sizeof(T));
	}

	void AddBack(T NewItem)
	{
		ExtendSize(sizeof(T));
		m_pArray[GetBackIndex() + 1] = NewItem;
	}

	size_t GetSize() const
	{
		return m_pPool->m_nSize;
	}

	// EDIT: I don't think this is right
	size_t GetBackIndex() const
	{
		// m_pPool / sizeof(T) = total size in bytes / bytes of time,
		// just like how you need to divide an offset in ida to get the real address
		return (m_pPool->m_nSize / sizeof(T));
	}

private:
	size_t m_nWishPoolSize;
};
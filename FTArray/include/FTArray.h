#pragma once
#include <random>
#include <thread>
#include <vector>
#include <utility>
#include <type_traits>

#include "ArrayIterator.h"
#include "Memory.h"
#include "Globals.h"

template<typename T>
class FTArray
{
private:
	static void Destruct(T* pMemory)
	{
		pMemory->~T();
	}

	static T* Construct(T* pMemory)
	{
		return ::new(pMemory) T;
	}

	static T* CopyConstruct(T* pMemory, T const& Src)
	{
		return ::new(pMemory) T(Src);
	}

	int InsertBefore(const int nIndex)
	{
		FT_ASSERT(nIndex == GetSize() || IsValidIndex(nIndex));

		Grow();
		ShiftRight(nIndex);
		Construct(&At(nIndex));

		return nIndex;
	}

	int InsertBefore(const int nIndex, const T& src)
	{
		FT_ASSERT(nIndex == GetSize() || IsValidIndex(nIndex));

		Grow();
		ShiftRight(nIndex);

		CopyConstruct(&At(nIndex), src);

		return nIndex;
	}

public:
	explicit FTArray(FTArray<T>& Other)
	{
		this->m_Memory = Other.m_Memory;
		this->m_nSize = Other.m_nSize;
		this->m_pElements = Other.m_pElements;

		m_bIsNumeric = std::is_arithmetic<T>();
	}

	FTArray(std::initializer_list<T> List)
	{
		for (auto Item : List)
			AddBack(Item);

		m_bIsNumeric = std::is_arithmetic<T>();
	}

	T* GetBase()
	{
		return m_Memory.Base();
	}

	const T* GetBase() const
	{
		return m_Memory.Base();
	}

	T& At(int nIndex)
	{
		return m_Memory[nIndex];
	}

	const T& At(int nIndex) const
	{
		return m_Memory[nIndex];
	}

	T& operator[](const int nIndex)
	{
		return At(nIndex);
	}

	const T& operator[](const int nIndex) const
	{
		return At(nIndex);
	}

	FTArray<T>& operator=(const FTArray<T>& Other)
	{
		if (this == Other)
			return *this;

		// Deallocate current memory
		for (int i = 0; i < m_nSize; i++)
		{
			Destruct(&At(i));
		}

		m_nSize = 0;
		m_Memory.Purge();
		m_pElements = nullptr;

		// Allocate new memory
		m_nSize = Other.GetSize();
		m_pElements = m_Memory.Alloc(m_nSize);

		// Copy construct elements
		for (int i = 0; i < m_nSize; i++)
			CopyConstruct(&At(i), Other.At(i));

		return *this;
	}

	bool IsValidIndex(const unsigned int nIndex) const
	{
		return nIndex < static_cast<unsigned int>(m_nSize);
	}

	void ShiftRight(const int nIndex, const int nNum = 1)
	{
		FT_ASSERT(IsValidIndex(nIndex) || m_nSize == 0 || nNum == 0);

		if (nNum <= 0)
			return;

		const int nNumToMove = m_nSize - nIndex - nNum;
		if (nNum <= 0)
			return;

		if (nNum == 1 && m_bIsNumeric)
		{
			const int* pSrc = (int*)&At(nIndex);
			int* pDest = (int*)&At(nIndex + nNum);
			for (int i = 0; i < nNumToMove; ++i)
				*pDest++ = *pSrc++;
		}
		else
			memmove(&At(nIndex + nNum), &At(nIndex), nNumToMove * sizeof(T));
	}

	void ShiftLeft(const int nIndex, const int nNum = 1)
	{
		FT_ASSERT(IsValidIndex(nIndex) || m_nSize == 0 || nNum == 0);

		if (nNum <= 0)
			return;

		const int nNumToMove = m_nSize - nIndex - nNum;
		if (nNum <= 0)
			return;

		if (nNum == 1 && m_bIsNumeric)
		{
			const int* pSrc = (int*)&At(nIndex + nNum);
			int* pDest = (int*)&At(nIndex);
			for (int i = 0; i < nNumToMove; ++i)
				*pDest++ = *pSrc++;
		}
		else
			memmove(&At(nIndex), &At(nIndex + nNum), nNumToMove * sizeof(T));
	}

	void Grow(const int nNum = 1)
	{
		const int nNewSize = m_nSize + nNum;
		if (nNewSize > m_Memory.GetAllocationCount())
			m_Memory.Grow(nNewSize - m_Memory.GetAllocationCount());

		m_nSize = nNewSize;
	}

	int AddFront()
	{
		return InsertBefore(0);
	}

	int AddFront(const T& Src)
	{
		return InsertBefore(0, Src);
	}

	int RemoveFront()
	{
		return Remove(0);
	}

	int RemoveFront(const T& Src)
	{
		return Remove(0, Src);
	}

	int AddBack()
	{
		return InsertBefore(m_nSize);
	}

	int AddBack(const T& Src)
	{
		return InsertBefore(m_nSize, Src);
	}

	int RemoveBack()
	{
		return Remove(m_nSize);
	}

	int RemoveBack(const T& Src)
	{
		return Remove(m_nSize, Src);
	}

	int Find(const T& Src)
	{
		for (int i = 0; i < GetSize(); i++)
		{
			if (At(i) == Src)
				return i;
		}

		return FT_INVALID_INDEX;
	}

	void Remove(const int nIndex)
	{
		Destruct(&At(nIndex));
		ShiftLeft(nIndex);
		m_nSize--;
	}

	FTArrayIterator<T> Begin()
	{
		return FTArrayIterator<T>(GetBase());
	}

	FTArrayIterator<T> End()
	{
		return FTArrayIterator<T>(GetBase() + GetSize());
	}

	int GetSize() const
	{
		return m_nSize;
	}

	/*
	 * Since std::shuffle already uses the efficient Fisher-Yates shuffle algorithm, there isn't
	 * much room for improvement in terms of algorithmic complexity, but there are a few small tweaks
	 * - Use std::move instead of std::swap for non-trivial types; Avoids unnecessary copying of objects
	 * - Use std::vector<bool> instead of std::vector<char> for flags, std::vector<bool> uses
	 *	 a specialized implementation that packs the booleans tightly, reducing memory usage
	 *	 and improving cache locality
	 * - Use auto for type deduction, reduces typing, especially when dealing with complex types
	 */
	void RandomShuffle(FTArrayIterator<T> First, FTArrayIterator<T> Last)
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		typedef typename std::iterator_traits<FTArrayIterator<T>>::difference_type diff_t;
		typedef std::uniform_int_distribution<diff_t> distr_t;
		typedef typename distr_t::param_type param_t;

		distr_t D;
		for (diff_t i = Last - First - 1; i > 0; --i)
		{
			if (std::is_trivial<decltype(*First)>())
				std::swap(First[i], First[D(gen, param_t(0, i))]);
			else
				std::iter_swap(First + i, First + D(gen, param_t(0, i)));
		}
	}

	/* This is the multi threaded version
	 * IMPORTANT: When nNumThreads is higher than the maximum supported be hardware,
	 * weird stuff happens, also happens if nNumThreads isn't even (except 1)
	 */
	void RandomShuffle(FTArrayIterator<T> First, FTArrayIterator<T> Last, int nNumThreads)
	{
		// nNumThreads is higher than supported by hardware
		assert(nNumThreads <= std::thread::hardware_concurrency());

		const std::ptrdiff_t nSize = Last - First;

		std::vector<std::thread> Threads;
		Threads.reserve(static_cast<size_t>(nNumThreads));

		std::vector<std::ptrdiff_t> ChunkSizes(nNumThreads);
		const std::ptrdiff_t nChunkSize = nSize / static_cast<std::ptrdiff_t>(nNumThreads);

		for (int i = 0; i < nNumThreads - 1; ++i)
			ChunkSizes[i] = nChunkSize;

		ChunkSizes[static_cast<std::vector<ptrdiff_t, std::allocator<ptrdiff_t>>::size_type>(
			nNumThreads) - 1] = nSize - nChunkSize * (static_cast<long long>(nNumThreads) - 1);

		std::vector<FTArrayIterator<T>> ChunkStarts(nNumThreads);
		ChunkStarts[0] = First;
		for (int i = 1; i < nNumThreads; ++i)
			ChunkStarts[i] =
			ChunkStarts[i - 1] + ChunkSizes[
				static_cast<std::vector<ptrdiff_t, std::allocator<ptrdiff_t>>::size_type>(i) - 1];

		std::random_device rd;
		std::mt19937 gen(rd());

		for (int i = 0; i < nNumThreads; ++i)
		{
			Threads.emplace_back([&ChunkStarts, &ChunkSizes, &gen, i]()
				{
					const FTArrayIterator<T> ChunkFirst = ChunkStarts[i];
					const FTArrayIterator<T> ChunkLast = ChunkFirst + ChunkSizes[i];

					std::uniform_int_distribution<std::ptrdiff_t> dist(0, ChunkSizes[i] - 1);
					for (std::ptrdiff_t j = ChunkSizes[i] - 1; j > 0; --j)
					{
						const std::ptrdiff_t k = dist(gen);

						if (std::is_trivial<decltype(ChunkFirst)>())
							std::swap(ChunkFirst[j], ChunkFirst[k]);
						else
							std::iter_swap(ChunkFirst + j, ChunkFirst + k);
					}
				});
		}

		for (std::thread& thread : Threads)
			thread.join();
	}

	void QuickSort(int nLow, int nHigh)
	{
		auto Partition = [](FTMemory<T>& arr, const int low, int high) -> int
		{
			T pivot = arr[high];
			int i = (low - 1);

			for (int j = low; j <= high - 1; j++)
			{
				if (arr[j] < pivot)
				{
					i++;
					std::swap(arr[i], arr[j]);
				}
			}

			std::swap(arr[i + 1], arr[high]);
			return (i + 1);
		};

		FT_ASSERT(IsTypeNumeric()); // This function is only for numbers
		FT_ASSERT(GetSize() < nHigh);

		if (nLow >= nHigh)
			return;

		const int nPivotPosition = Partition(m_Memory, nLow, nHigh);

		QuickSort(nLow, nPivotPosition - 1);
		QuickSort(nPivotPosition + 1, nHigh);
	}

	void QuickSort()
	{
		FT_ASSERT(IsTypeNumeric()); // This function is only for numbers
		QuickSort(0, GetSize() - 1);
	}

private:
	FTMemory<T> m_Memory;
	int m_nSize = 0;
	T* m_pElements = nullptr;
	bool m_bIsNumeric = false;
};
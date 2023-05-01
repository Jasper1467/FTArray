#pragma once
#include <cassert>
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
		assert(nIndex == GetSize() || IsValidIndex(nIndex));

		Grow();
		ShiftRight(nIndex);
		Construct(&At(nIndex));
		return nIndex;
	}

	int InsertBefore(int nIndex, const T& src)
	{
		assert(nIndex == GetSize() || IsValidIndex(nIndex));

		Grow();
		ShiftRight(nIndex);
		CopyConstruct(&At(nIndex), src);
		return nIndex;
	}

public:
	FTArray(std::initializer_list<T> List)
	{
		for (auto Item : List)
			AddBack(Item);
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

	const T& operator[](int nIndex) const
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

	bool IsValidIndex(const int nIndex) const
	{
		return (nIndex >= 0) && (nIndex < m_nSize);
	}

	void ShiftRight(const int nIndex, int nNum = 1)
	{
		assert(IsValidIndex(nIndex) || m_nSize == 0 || nNum == 0);

		const int nNumToMove = m_nSize - nIndex - nNum;
		if ((nNumToMove > 0) && (nNum > 0))
			memmove(&At(nIndex + nNum), &At(nIndex), nNumToMove * sizeof(T));
	}

	void ShiftLeft(const int nIndex, int nNum = 1)
	{
		assert(IsValidIndex(nIndex) || m_nSize == 0 || nNum == 0);

		const int nNumToMove = m_nSize - nIndex - nNum;
		if ((nNumToMove > 0) && (nNum > 0))
			memmove(&At(nIndex), &At(nIndex + nNum), nNumToMove * sizeof(T));
	}

	void Grow(const int nNum = 1)
	{
		if (m_nSize + nNum > m_Memory.GetAllocationCount())
			m_Memory.Grow(m_nSize + nNum - m_Memory.GetAllocationCount());

		m_nSize += nNum;
	}

	int AddBegin()
	{
		return InsertBefore(0);
	}

	int AddBegin(const T& Src)
	{
		return InsertBefore(0, Src);
	}

	int AddBack()
	{
		return InsertBefore(m_nSize);
	}

	int AddBack(const T& Src)
	{
		return InsertBefore(m_nSize, Src);
	}

	int Find(const T& Src)
	{
		for (int i = 0; i < GetSize(); i++)
		{
			if (At(i) == Src)
				return i;
		}

		return details::INVALID_INDEX;
	}

	void Remove(const int nIndex)
	{
		Destruct(&At(nIndex));
		ShiftLeft(nIndex);
		m_nSize--;
	}

	FTArrayIterator<T> Begin()
	{
		return GetBase();
	}

	FTArrayIterator<T> End()
	{
		return GetBase() + GetSize();
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

	// This is the multi threaded version
	void RandomShuffle(FTArrayIterator<T> First, FTArrayIterator<T> Last, int nNumThreads)
	{
		const std::ptrdiff_t nSize = Last - First;

		std::vector<std::thread> Threads;
		Threads.reserve(nNumThreads);

		std::vector<std::ptrdiff_t> ChunkSizes(nNumThreads);
		const std::ptrdiff_t nChunkSize = nSize / nNumThreads;

		for (int i = 0; i < nNumThreads - 1; ++i)
			ChunkSizes[i] = nChunkSize;

		ChunkSizes[nNumThreads - 1] = nSize - nChunkSize * (nNumThreads - 1);

		std::vector<FTArrayIterator<T>> ChunkStarts(nNumThreads);
		ChunkStarts[0] = First;
		for (int i = 1; i < nNumThreads; ++i)
			ChunkStarts[i] = ChunkStarts[i - 1] + ChunkSizes[i - 1];

		typedef typename std::iterator_traits<FTArrayIterator<T>>::difference_type diff_t;
		typedef std::uniform_int_distribution<diff_t> distr_t;
		typedef typename distr_t::param_type param_t;

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
							std::swap(ChunkFirst[i], ChunkFirst[k]);
						else
							std::iter_swap(ChunkFirst + i, ChunkFirst + k);
					}
				});
		}

		for (std::thread& thread : Threads)
			thread.join();
	}

private:
	FTMemory<T> m_Memory;
	int m_nSize = 0;
	T* m_pElements = nullptr;
};
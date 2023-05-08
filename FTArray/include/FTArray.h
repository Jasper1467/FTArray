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
	__forceinline static void Destruct(T* pMemory) noexcept
	{
		pMemory->~T();
	}

	__forceinline static T* Construct(T* pMemory) noexcept
	{
		return ::new(pMemory) T;
	}

	__forceinline static T* CopyConstruct(T* pMemory, T const& Src) noexcept
	{
		return ::new(pMemory) T(Src);
	}

	__forceinline int InsertBefore(const int nIndex) noexcept
	{
		FT_ASSERT(nIndex == GetSize() || IsValidIndex(nIndex));

		Grow();
		ShiftRight(nIndex);
		Construct(&At(nIndex));

		return nIndex;
	}

	__forceinline int InsertBefore(const int nIndex, const T& src) noexcept
	{
		FT_ASSERT(nIndex == GetSize() || IsValidIndex(nIndex));

		Grow();
		ShiftRight(nIndex);

		CopyConstruct(&At(nIndex), src);

		return nIndex;
	}

public:
	__forceinline explicit FTArray(FTArray<T>& Other) noexcept
	{
		this->m_Memory = Other.m_Memory;
		this->m_nSize = Other.m_nSize;
		this->m_pElements = Other.m_pElements;

		m_bIsNumeric = std::is_arithmetic<T>();
	}

	__forceinline FTArray(std::initializer_list<T> List) noexcept
	{
		for (auto Item : List)
			AddBack(Item);

		m_bIsNumeric = std::is_arithmetic<T>();
	}

	__forceinline T* GetBase() noexcept
	{
		return m_Memory.Base();
	}

	__forceinline const T* GetBase() const noexcept
	{
		return m_Memory.Base();
	}

	__forceinline T& At(int nIndex) noexcept
	{
		return m_Memory[nIndex];
	}

	__forceinline const T& At(int nIndex) const noexcept
	{
		return m_Memory[nIndex];
	}

	__forceinline T& operator[](const int nIndex) noexcept
	{
		return At(nIndex);
	}

	__forceinline const T& operator[](const int nIndex) const noexcept
	{
		return At(nIndex);
	}

	__forceinline FTArray<T>& operator=(const FTArray<T>& Other) noexcept
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

	__forceinline bool IsValidIndex(const unsigned int nIndex) const noexcept
	{
		return nIndex < static_cast<unsigned int>(m_nSize);
	}

	__forceinline void ShiftRight(const int nIndex, const int nNum = 1)
	{
		FT_ASSERT(IsValidIndex(nIndex) || m_nSize == 0 || nNum == 0);

		if (nNum <= 0)
			return;

		const int nNumToMove = m_nSize - nIndex - nNum;
		if (nNumToMove <= 0)
			return;

		if (nNum == 1 && m_bIsNumeric)
		{
			const int* pSrc = static_cast<int*>(&At(nIndex));
			int* pDest = static_cast<int*>(&At(nIndex + nNum));
			for (int i = 0; i < nNumToMove; ++i)
				*pDest++ = *pSrc++;
		}
		else
			memmove(&At(nIndex + nNum), &At(nIndex), nNumToMove * sizeof(T));
	}

	__forceinline void ShiftLeft(const int nIndex, const int nNum = 1) noexcept
	{
		FT_ASSERT(IsValidIndex(nIndex) || m_nSize == 0 || nNum == 0);

		if (nNum <= 0)
			return;

		const int nNumToMove = m_nSize - nIndex - nNum;
		if (nNumToMove <= 0)
			return;

		if (nNum == 1 && m_bIsNumeric)
		{
			const int* pSrc = static_cast<int*>(&At(nIndex + nNum));
			int* pDest = static_cast<int*>(&At(nIndex));
			for (int i = 0; i < nNumToMove; ++i)
				*pDest++ = *pSrc++;
		}
		else
			memmove(&At(nIndex), &At(nIndex + nNum), nNumToMove * sizeof(T));
	}

	__forceinline void Grow(const int nNum = 1) noexcept
	{
		const int nNewSize = m_nSize + nNum;
		if (nNewSize > m_Memory.GetAllocationCount())
			m_Memory.Grow(nNewSize - m_Memory.GetAllocationCount());

		m_nSize = nNewSize;
	}

	__forceinline int AddFront() noexcept
	{
		return InsertBefore(0);
	}

	__forceinline int AddFront(const T& Src) noexcept
	{
		return InsertBefore(0, Src);
	}

	__forceinline int RemoveFront() noexcept
	{
		return Remove(0);
	}

	__forceinline int RemoveFront(const T& Src) noexcept
	{
		return Remove(0, Src);
	}

	__forceinline int AddBack() noexcept
	{
		return InsertBefore(m_nSize);
	}

	__forceinline int AddBack(const T& Src) noexcept
	{
		return InsertBefore(m_nSize, Src);
	}

	__forceinline int RemoveBack() noexcept
	{
		return Remove(m_nSize);
	}

	__forceinline int RemoveBack(const T& Src) noexcept
	{
		return Remove(m_nSize, Src);
	}

	__forceinline int Find(const T& Src) noexcept
	{
		for (int i = 0; i < GetSize(); i++)
		{
			if (At(i) == Src)
				return i;
		}

		return FT_INVALID_INDEX;
	}

	__forceinline void Remove(const int nIndex) noexcept
	{
		Destruct(&At(nIndex));
		ShiftLeft(nIndex);
		m_nSize--;
	}

	__forceinline FTArrayIterator<T> Begin() noexcept
	{
		return FTArrayIterator<T>(GetBase());
	}

	__forceinline FTArrayIterator<T> End() noexcept
	{
		return FTArrayIterator<T>(GetBase() + GetSize());
	}

	__forceinline int GetSize() const noexcept
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
	__forceinline void RandomShuffle(FTArrayIterator<T> First, FTArrayIterator<T> Last) noexcept
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
	__forceinline void RandomShuffle(FTArrayIterator<T> First, FTArrayIterator<T> Last, int nNumThreads) noexcept
	{
		// nNumThreads is higher than supported by hardware
		assert(nNumThreads <= std::thread::hardware_concurrency());

		const size_t nSize = Last - First;

		std::vector<std::thread> Threads;
		Threads.reserve(static_cast<size_t>(nNumThreads));

		std::vector<size_t> ChunkSizes(nNumThreads);
		const size_t nChunkSize = nSize / static_cast<std::size_t>(nNumThreads);

		for (int i = 0; i < nNumThreads - 1; ++i)
			ChunkSizes[i] = nChunkSize;

		ChunkSizes[static_cast<std::vector<size_t, std::allocator<size_t>>::size_type>(
			nNumThreads) - 1] = nSize - nChunkSize * (static_cast<long long>(nNumThreads) - 1);

		std::vector<FTArrayIterator<T>> ChunkStarts(nNumThreads);
		ChunkStarts[0] = First;
		for (int i = 1; i < nNumThreads; ++i)
			ChunkStarts[i] =
			ChunkStarts[i - 1] + ChunkSizes[
				static_cast<std::vector<size_t, std::allocator<size_t>>::size_type>(i) - 1];

		std::random_device rd;
		std::mt19937 gen(rd());

		for (int i = 0; i < nNumThreads; ++i)
		{
			Threads.emplace_back([&ChunkStarts, &ChunkSizes, &gen, i]()
				{
					const FTArrayIterator<T> ChunkFirst = ChunkStarts[i];
					const FTArrayIterator<T> ChunkLast = ChunkFirst + ChunkSizes[i];

					std::uniform_int_distribution<size_t> dist(0, ChunkSizes[i] - 1);
					for (size_t j = ChunkSizes[i] - 1; j > 0; --j)
					{
						const size_t k = dist(gen);

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

	__forceinline void QuickSort(int nLow, int nHigh) noexcept
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

	__forceinline void QuickSort() noexcept
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
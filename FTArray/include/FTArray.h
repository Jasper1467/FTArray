#pragma once
#include <cassert>
#include <random>
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
		assert(IsValidIndex(nIndex) || !m_nSize || !nNum);

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
		std::uniform_int_distribution<std::ptrdiff_t> dist;

		std::vector<bool> Used(Last - First, false);
		for (std::ptrdiff_t i = Last - First - 1; i > 0; --i)
		{
			auto j = dist(gen, decltype(dist)::param_type(0, i));
			while (Used[j])
				j = dist(gen, decltype(dist)::param_type(0, i));

			Used[j] = true;

			if constexpr (std::is_trivially_copyable_v<decltype(*First)>)
				std::swap(First[i], First[j]);
			else
				std::iter_swap(First + i, First + j);
		}
	}

private:
	FTMemory<T> m_Memory;
	int m_nSize = 0;
	T* m_pElements = nullptr;
};
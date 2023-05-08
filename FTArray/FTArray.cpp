#include <chrono>

#include "include/FTArray.h"

double mticks()
{
	typedef std::chrono::high_resolution_clock clock;
	typedef std::chrono::duration<float, std::milli> duration;

	static clock::time_point start = clock::now();
	const duration elapsed = clock::now() - start;
	return elapsed.count();
}

#define TEST_AMOUNT 50000

double g_dbResults[TEST_AMOUNT] = {};

void Benchmark(const int nTestNum)
{
	FTArray<int> Array = {};
	std::vector<int> Vector = {};

	typedef std::chrono::high_resolution_clock clock;
	typedef std::chrono::duration<float, std::milli> duration;

	const clock::time_point VectorStart = clock::now();
	while (Vector.size() != 10000)
	{
		Vector.push_back(1);
	}
	const duration VectorDuration = clock::now() - VectorStart;

	printf("%i: Vector took %f milliseconds\n", nTestNum, VectorDuration.count());

		const clock::time_point ArrayStart = clock::now();
	while (Array.GetSize() != 10000)
	{
		Array.AddBack(1);
	}
	const duration ArrayDuration = clock::now() - ArrayStart;

	printf("%i: FTArray took %f milliseconds\n", nTestNum, ArrayDuration.count());

	g_dbResults[nTestNum] = VectorDuration.count() + ArrayDuration.count();
}

int main()
{
	for (int i = 0; i < TEST_AMOUNT; i++)
	{
		Benchmark(i);
	}

	double dbAverage = 0.0;
	for (int i = 0; i < TEST_AMOUNT; i++)
	{
		dbAverage += g_dbResults[i];
	}

	dbAverage /= TEST_AMOUNT;

	if (dbAverage < 0.0)
		printf("Vector was on average %f faster\n", dbAverage);
	else
		printf("FTArray was on average %f faster\n", dbAverage);
}
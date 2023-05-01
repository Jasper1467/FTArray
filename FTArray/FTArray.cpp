#include <iostream>

#include "include/FTArray.h"

int main()
{
    FTArray<int> Numbers = { 1, 2, 3, 4, 5 };
    Numbers.AddBack(6);
    Numbers.AddBack(7);
    Numbers.AddBack(8);
    Numbers.AddBack(9);
    Numbers.AddBack(10);

    Numbers.RandomShuffle(Numbers.Begin(), Numbers.End(), 4);

     for (size_t i = 0; i < Numbers.GetSize(); i++)
        printf("%i\n", Numbers[i]);
}
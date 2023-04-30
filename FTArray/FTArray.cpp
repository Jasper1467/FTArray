#include <iostream>

#include "include/FTArray.h"

int main()
{
    FTArray<int> Numbers;
    Numbers.AddBack(1);
    Numbers.AddBack(2);
    Numbers.AddBack(3);
    Numbers.AddBack(4);
    Numbers.AddBack(5);

    Numbers.RandomShuffle(Numbers.Begin(), Numbers.End());

     for (size_t i = 0; i < Numbers.GetSize(); i++)
        printf("%i\n", Numbers[i]);
}
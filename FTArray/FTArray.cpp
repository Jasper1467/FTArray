#include <iostream>

#include "include/FTArray.h"

int main()
{
    FTArray<int, 5> Numbers;
    Numbers.AddBack(1);
    Numbers.AddBack(2);
    Numbers.AddBack(3);
    Numbers.AddBack(4);
    Numbers.AddBack(5);

    for (int i = 1; i <= 5; i++)
        printf("%i\n", Numbers[i]);
}
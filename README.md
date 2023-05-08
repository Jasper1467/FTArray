# FTArray
FTArray aims to efficiently manage and manipulate large arrays of data in C++. It was designed with performance and memory efficiency in mind.

# Benchmark
I used the code from FTArray.cpp to benchmark, it calculates the time used for a for loop from 0 to 10000 to complete. I did the test 50000 times and calculated the average.
**FTArray was on average 2.031036ms faster than std::vector in x64 and 0.789200ms in x86. Please note that these tests were done on Release and not Debug, Debug has asserts which leads to a decrease in performance**

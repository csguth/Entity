#include <iostream>

#include "TupleVectorBenchmark.hpp"
#include "TupleVector.hpp"

template <uint32_t N, class Callable>
void repeat(Callable cb)
{
    for(volatile int i = 0; i < N; ++i)
    {
        cb();
    }
}

void tupleVector()
{
    using namespace Entity;
    TupleVector<uint32_t, uint8_t, uint64_t> vec;
    repeat<10000000>([&]()
    {
       vec.resize(vec.size() + 1);
    });
    std::cout << "capacity = " << vec.capacity() << " size = " << vec.size() << std::endl;
}

void baseline()
{
    std::vector<uint32_t> vec0;
    std::vector<uint8_t>  vec1;
    std::vector<uint64_t> vec2;
    repeat<10000000>([&]()
    {
        vec0.resize(vec0.size() + 1);
        vec1.resize(vec1.size() + 1);
        vec2.resize(vec2.size() + 1);
    });
    std::cout << "capacity = " << vec0.capacity() << " size = " << vec0.size() << std::endl;
}

#include <chrono>

template <class Callable>
void profile(const char testName[], Callable test)
{
    using namespace std::chrono;
    auto const start = high_resolution_clock::now();
    test();
    auto const end   = high_resolution_clock::now();
    std::cout << testName << " took " << duration_cast<milliseconds>(end-start).count() << " ms" << std::endl;
}

int main(int, char*[])
{
    profile("Baseline", baseline);
    profile("Tuple Vector", tupleVector);
    return 0;
}




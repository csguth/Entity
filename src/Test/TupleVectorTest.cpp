#include <catch.hpp>
#include <Entity/Core/TupleVector.hpp>

using namespace Entity;

TEST_CASE("Empty tuple vector", "[TupleVector]")
{
    {
        TupleVector<int> vector;
        CHECK(vector.empty());
        CHECK(vector.size() == 0);
    }
    {
        TupleVector<int, double> vector;
        CHECK(vector.empty());
        CHECK(vector.size() == 0);
    }
    {
        TupleVector<int, double, int> vector;
        CHECK(vector.empty());
        CHECK(vector.size() == 0);
    }
}

TEST_CASE("Words needed", "[TupleVector]")
{
    auto wordsNeededZero = [](std::size_t i)
    {
      return TupleVectorTraits<0, uint8_t>::wordsNeeded(i);
    };

    auto wordsNeededOne1Byte = [](std::size_t i)
    {
      return TupleVectorTraits<1, uint8_t>::wordsNeeded(i);
    };

    auto wordsNeededOne4Bytes = [](std::size_t i)
    {
      return TupleVectorTraits<1, uint32_t>::wordsNeeded(i);
    };

    auto wordsNeededTwo = [](std::size_t i)
    {
      return TupleVectorTraits<2, uint8_t, uint32_t>::wordsNeeded(i);
    };

    CHECK(wordsNeededZero(0) == 0);
    CHECK(wordsNeededOne1Byte(0) == 0);
    CHECK(wordsNeededOne1Byte(1) == 1);
    CHECK(wordsNeededOne1Byte(2) == 1);
    CHECK(wordsNeededOne1Byte(3) == 1);
    CHECK(wordsNeededOne1Byte(4) == 1);
    CHECK(wordsNeededOne1Byte(5) == 2);
    CHECK(wordsNeededOne1Byte(6) == 2);
    CHECK(wordsNeededOne1Byte(7) == 2);
    CHECK(wordsNeededOne1Byte(8) == 2);
    CHECK(wordsNeededOne1Byte(9) == 3);
    CHECK(wordsNeededOne4Bytes(1) == 1);
    CHECK(wordsNeededOne4Bytes(2) == 2);
    CHECK(wordsNeededOne4Bytes(3) == 3);
    CHECK(wordsNeededTwo(0) == 0);
    CHECK(wordsNeededTwo(1) == 2);
    CHECK(wordsNeededTwo(2) == 1 + 2);
    CHECK(wordsNeededTwo(3) == 1 + 3);
    CHECK(wordsNeededTwo(4) == 1 + 4);
    CHECK(wordsNeededTwo(5) == 2 + 5);
}

template<uint32_t offset>
auto firstWord(std::size_t capacity)
{
    return TupleVectorTraits<offset, uint32_t, uint8_t, uint64_t>::firstWord(capacity);
}

template<uint32_t offset>
auto lastWord(std::size_t capacity)
{
    return TupleVectorTraits<offset, uint32_t, uint8_t, uint64_t>::lastWord(capacity);
}

TEST_CASE("First word index", "[TupleVector]")
{
    CHECK(firstWord<1>(0)    == 0    + 0   + 0);
    CHECK(firstWord<1>(42)   == 0    + 0   + 0);
    CHECK(firstWord<1>(1024) == 0    + 0   + 0);

    CHECK(firstWord<2>(0)    == 0    + 0   + 0);
    CHECK(firstWord<2>(42)   == 42   + 0   + 0);
    CHECK(firstWord<2>(1024) == 1024 + 0   + 0);

    CHECK(firstWord<3>(0)    == 0    + 0   + 0);
    CHECK(firstWord<3>(42)   == 42   + 11  + 0);
    CHECK(firstWord<3>(1024) == 1024 + 256 + 0);
}

TEST_CASE("Last word index", "[TupleVector]")
{
    CHECK(lastWord<1>(0)    == 0    + 0   + 0   );
    CHECK(lastWord<1>(42)   == 42   + 0   + 0   );
    CHECK(lastWord<1>(1024) == 1024 + 0   + 0   );

    CHECK(lastWord<2>(0)    == 0    + 0   + 0   );
    CHECK(lastWord<2>(42)   == 42   + 11  + 0   );
    CHECK(lastWord<2>(1024) == 1024 + 256 + 0   );

    CHECK(lastWord<3>(0)    == 0    + 0   + 0   );
    CHECK(lastWord<3>(42)   == 42   + 11  + 84  );
    CHECK(lastWord<3>(1024) == 1024 + 256 + 2048);
}

TEST_CASE("Access", "[TupleVector]")
{
    {
        TupleVector<int> vector(3);
        vector.set<0>(0, 1);
        vector.set<0>(1, 2);
        vector.set<0>(2, 3);
        const auto& constVector = vector;
        CHECK(constVector.at<0>(0) == 1);
        CHECK(constVector.at<0>(1) == 2);
        CHECK(constVector.at<0>(2) == 3);
    }
    {
        TupleVector<int, double> vector(2);
        vector.set<0>(0, 1);
        vector.set<0>(1, 2);
        vector.set<1>(0, .1);
        vector.set<1>(1, .2);
        const auto& constVector = vector;
        CHECK(constVector.at<0>(0) == 1);
        CHECK(constVector.at<0>(1) == 2);
        CHECK(constVector.at<1>(0) == .1);
        CHECK(constVector.at<1>(1) == .2);
    }
    {
        TupleVector<int, char, double> vector(2);
        vector.set<0>(0, 1);
        vector.set<0>(1, 2);
        vector.set<1>(0, 'a');
        vector.set<1>(1, 'b');
        vector.set<2>(0, .1);
        vector.set<2>(1, .2);
        const auto& constVector = vector;
        CHECK(constVector.at<0>(0) == 1);
        CHECK(constVector.at<0>(1) == 2);
        CHECK(constVector.at<1>(0) == 'a');
        CHECK(constVector.at<1>(1) == 'b');
        CHECK(constVector.at<2>(0) == .1);
        CHECK(constVector.at<2>(1) == .2);
    }
}


TEST_CASE("Reserve & Resize", "[TupleVector]")
{
    auto check = [](auto&& vector)
    {
        CHECK(vector.capacity() == 0);
        vector.reserve(1024);
        CHECK(vector.capacity() == 1024);
        vector.resize(1000);
        CHECK(vector.size() == 1000);
        CHECK(vector.capacity() == 1024);
        vector.resize(0);
        CHECK(vector.size() == 0);
        CHECK(vector.capacity() == 1024);
        vector.resize(100);
        CHECK(vector.size() == 100);
        CHECK(vector.capacity() == 1024);
        vector.clear();
        CHECK(vector.size() == 0);
        CHECK(vector.capacity() == 0);
    };
    {
        TupleVector<int> vector;
        check(vector);
    }
    {
        TupleVector<int, double> vector;
        check(vector);
    }
    {
        TupleVector<int, double, int> vector;
        check(vector);
    }
}

TEST_CASE("Copy", "[TupleVector]")
{
    {
        std::vector<uint32_t> origin{{1, 2}};
        std::vector<uint32_t> destination{{0, 0, 0}};
        CHECK(destination.size() == 3);
        TupleVectorTraits<1, uint32_t>::copy(origin, 2, destination, 3);
        const std::vector<uint32_t> goldenResult{{1, 2, 0}};
        CHECK(destination == goldenResult);
    }
    {
        std::vector<uint32_t> origin{{1, 2, 3, 4}};
        std::vector<uint32_t> destination{{0, 0, 0, 0, 0, 0}};
        TupleVectorTraits<1, uint32_t, uint32_t>::copy(origin, 2, destination, 3);
        const std::vector<uint32_t> goldenResult{{1, 2, 0, 0, 0, 0}};
        CHECK(destination == goldenResult);
    }
    {
        std::vector<uint32_t> origin{{1, 2, 3, 4}};
        std::vector<uint32_t> destination{{0, 0, 0, 0, 0, 0}};
        TupleVectorTraits<2, uint32_t, uint32_t>::copy(origin, 2, destination, 3);
        const std::vector<uint32_t> goldenResult{{1, 2, 0, 3, 4, 0}};
        CHECK(destination == goldenResult);
    }
    {
        std::vector<uint32_t> origin{{1, 2, 3, 4}};
        std::vector<uint32_t> destination{{0, 0, 0, 0, 0, 0, 0, 0}};
        TupleVectorTraits<1, uint32_t, uint32_t>::copy(origin, 2, destination, 4);
        const std::vector<uint32_t> goldenResult{{1, 2, 0, 0, 0, 0, 0, 0}};
        CHECK(destination == goldenResult);
    }
    {
        std::vector<uint32_t> origin{{0x00000001, 0x00000002, 0x00000403}};
        std::vector<uint32_t> destination{{0, 0, 0, 0, 0}};
        TupleVectorTraits<2, uint32_t, uint8_t>::copy(origin, 2, destination, 4);
        const std::vector<uint32_t> goldenResult{{0x00000001, 0x00000002, 0, 0, 0x00000403}};
        CHECK(destination == goldenResult);
    }

}

TEST_CASE("Reallocation keeps elements on their positions", "[TupleVector]")
{
    //[   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25]
    //[32_0,32_0,32_0,32_0,32_1,32_1,32_1,32_1, 8_0, 8_1,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_1]
    TupleVector<int32_t, int8_t, int64_t> vector(2);

    vector.set<0>(0, 0xCCCCCCFF);
    vector.set<0>(1, 0xCCCCFFCC);
    vector.set<1>(0, 'c');
    vector.set<1>(1, 'd');
    vector.set<2>(0, 0xCCCCCCCCFFFFFFFFul);
    vector.set<2>(1, 0xFFFFFFFFCCCCCCCCul);

    vector.resize(3);

    {
        CHECK(vector.at<0>(0) == 0xCCCCCCFF);
        CHECK(vector.at<0>(1) == 0xCCCCFFCC);
        CHECK(vector.at<1>(0) == 'c');
        CHECK(vector.at<1>(1) == 'd');
        CHECK(vector.at<2>(0) == 0xCCCCCCCCFFFFFFFFul);
        CHECK(vector.at<2>(1) == 0xFFFFFFFFCCCCCCCCul);
    }

    CHECK(vector.size() == 3);
    CHECK(vector.capacity() == 4);

//    //[   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38]
//    //[32_0,32_0,32_0,32_0,32_1,32_1,32_1,32_1,32_2,32_2,32_2,32_2, 8_0, 8_1, 8_2,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_2,64_2,64_2,64_2,64_2,64_2,64_2,64_2]
    const auto& constVector = vector;
    CHECK(constVector.at<0>(0) == 0xCCCCCCFF);
    CHECK(constVector.at<0>(1) == 0xCCCCFFCC);
    CHECK(constVector.at<1>(0) == 'c');
    CHECK(constVector.at<1>(1) == 'd');
    CHECK(constVector.at<2>(0) == 0xCCCCCCCCFFFFFFFF);
    CHECK(constVector.at<2>(1) == 0xFFFFFFFFCCCCCCCC);
    vector.set<0>(2, 0xAAAAAAAA);
    vector.set<1>(2, 'b');
    vector.set<2>(2, 0xDDDDDDDDEEEEEEEE);
    CHECK(constVector.at<0>(0) == 0xCCCCCCFF);
    CHECK(constVector.at<0>(1) == 0xCCCCFFCC);
    CHECK(constVector.at<0>(2) == 0xAAAAAAAA);
    INFO(+constVector.at<1>(0));
    CHECK(constVector.at<1>(0) == 'c');
    INFO(+constVector.at<1>(1));
    CHECK(constVector.at<1>(1) == 'd');
    INFO(+constVector.at<1>(2));
    CHECK(constVector.at<1>(2) == 'b');
    CHECK(constVector.at<2>(0) == 0xCCCCCCCCFFFFFFFF);
    CHECK(constVector.at<2>(1) == 0xFFFFFFFFCCCCCCCC);
    CHECK(constVector.at<2>(2) == 0xDDDDDDDDEEEEEEEE);

}

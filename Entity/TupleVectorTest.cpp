#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "TupleVector.hpp"

using namespace Entity;

TEST_CASE("Empty tuple vector")
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


TEST_CASE("Reserve & Resize")
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

TEST_CASE("Begin byte traits")
{
    std::vector<std::tuple<int32_t, int8_t, int64_t>> vector;
    {
        auto intBeginByte     = TupleVectorTraits<0, int32_t, int8_t, int64_t>::beginByte(vector);
        auto charBeginByte    = TupleVectorTraits<1, int32_t, int8_t, int64_t>::beginByte(vector);
        auto doubleBeginByte  = TupleVectorTraits<2, int32_t, int8_t, int64_t>::beginByte(vector);
        CHECK(intBeginByte    == 0);
        CHECK(charBeginByte   == 0);
        CHECK(doubleBeginByte == 0);
    }
    vector.reserve(2);
    vector.resize(1);
    {
        //[   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25]
        //[32_0,32_0,32_0,32_0,32_1,32_1,32_1,32_1, 8_0, 8_1,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_1]
        auto intBeginByte     = TupleVectorTraits<0, int32_t, int8_t, int64_t>::beginByte(vector);
        auto charBeginByte    = TupleVectorTraits<1, int32_t, int8_t, int64_t>::beginByte(vector);
        auto doubleBeginByte  = TupleVectorTraits<2, int32_t, int8_t, int64_t>::beginByte(vector);
        CHECK(intBeginByte    == 0);
        CHECK(charBeginByte   == 8);
        CHECK(doubleBeginByte == 10);

        auto intEndByte     = TupleVectorTraits<0, int32_t, int8_t, int64_t>::endByte(vector);
        auto charEndByte    = TupleVectorTraits<1, int32_t, int8_t, int64_t>::endByte(vector);
        auto doubleEndByte  = TupleVectorTraits<2, int32_t, int8_t, int64_t>::endByte(vector);
        CHECK(intEndByte    == 4);
        CHECK(charEndByte   == 9);
        CHECK(doubleEndByte == 18);
    }
}

TEST_CASE("Access")
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
        TupleVector<int, double, char> vector(2);
        vector.set<0>(0, 1);
        vector.set<0>(1, 2);
        vector.set<1>(0, .1);
        vector.set<1>(1, .2);
        vector.set<2>(0, 'a');
        vector.set<2>(1, 'b');
        const auto& constVector = vector;
        CHECK(constVector.at<0>(0) == 1);
        CHECK(constVector.at<0>(1) == 2);
        CHECK(constVector.at<1>(0) == .1);
        CHECK(constVector.at<1>(1) == .2);
        CHECK(constVector.at<2>(0) == 'a');
        CHECK(constVector.at<2>(1) == 'b');
    }
}

TEST_CASE("Realocation keeps elements on their positions")
{
    //[   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25]
    //[32_0,32_0,32_0,32_0,32_1,32_1,32_1,32_1, 8_0, 8_1,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_1]
    TupleVector<int32_t, int8_t, int64_t> vector(2);
    vector.set<0>(0, 1);
    vector.set<0>(1, 2);
    vector.set<1>(0, 3);
    vector.set<1>(1, 4);
    vector.set<2>(0, 5);
    vector.set<2>(1, 6);
    vector.resize(3);
    //[   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38]
    //[32_0,32_0,32_0,32_0,32_1,32_1,32_1,32_1,32_2,32_2,32_2,32_2, 8_0, 8_1, 8_2,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_0,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_1,64_2,64_2,64_2,64_2,64_2,64_2,64_2,64_2]
    const auto& constVector = vector;
    CHECK(constVector.at<0>(0) == 1);
    CHECK(constVector.at<0>(1) == 2);
    CHECK(constVector.at<1>(0) == 3);
    CHECK(constVector.at<1>(1) == 4);
    CHECK(constVector.at<2>(0) == 5);
    CHECK(constVector.at<2>(1) == 6);
    vector.set<0>(2, 7);
    vector.set<1>(2, 8);
    vector.set<2>(2, 9);
    CHECK(constVector.at<0>(0) == 1);
    CHECK(constVector.at<0>(1) == 2);
    CHECK(constVector.at<0>(2) == 7); //
    CHECK(constVector.at<1>(0) == 3);
    CHECK(constVector.at<1>(1) == 4);
    CHECK(constVector.at<1>(2) == 8); //
    CHECK(constVector.at<2>(0) == 5);
    CHECK(constVector.at<2>(1) == 6);
    CHECK(constVector.at<2>(2) == 9); //
}

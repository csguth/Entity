#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <algorithm>
#include "Property.hpp"

using namespace Entity;
TEST_CASE("Property")
{
    System<Base> sys;
    auto prop = makeProperty<Base, double>(sys);
    CHECK(prop.empty());
    CHECK(prop.size() == sys.size());
}

TEST_CASE("Add After")
{
    System<Base> sys;
    auto prop = makeProperty<Base, double>(sys);
    const Base en = sys.add();
    CHECK(prop.size() == sys.size());
    CHECK(prop.size() == 1);
}

TEST_CASE("Add Before")
{
    System<Base> sys;
    const Base en = sys.add();
    auto prop = makeProperty<Base, double>(sys);
    CHECK(prop.size() == sys.size());
    CHECK(prop.size() == 1);
}

TEST_CASE("Capacity After")
{
    System<Base> sys;
    auto prop = makeProperty<Base, double>(sys);
    sys.reserve(1024);
    CHECK(sys.capacity() == prop.capacity());
    CHECK(prop.capacity() == 1024);
}

TEST_CASE("Capacity Before")
{
    System<Base> sys;
    sys.reserve(1024);
    auto prop = makeProperty<Base, double>(sys);
    CHECK(sys.capacity() == prop.capacity());
    CHECK(prop.capacity() == 1024);
}

TEST_CASE("Scoped Connections")
{
    System<Base> sys;
    {
        auto prop = makeProperty<Base, double>(sys);
        sys.add();
        sys.add();
        sys.add();
    }
    CHECK_NOTHROW(sys.add());
}

using namespace ranges;
#include <iostream>
TEST_CASE("As Range")
{
    System<Base> sys;
    auto prop = makeProperty<Base, double>(sys);
    prop[sys.add()] = 42.0;
    prop[sys.add()] = 84.0;
    auto range = prop.asRange();
    for_each(view::zip(sys.asRange(), prop.asRange()), [](std::pair<Base, double&> el){ // Should modify
        static double count = 1.0;
        el.second = 66.0 * count;
        count += 1.0;
    });

    for_each(view::zip(sys.asRange(), prop.asRange()), [](std::pair<Base, double> el){ // Should not modify
        static double count = 1.0;
        el.second = 77.0 * count;
        count += 1.0;
    });

    std::vector<double> result;
    for_each(view::zip(sys.asRange(), prop.asRange()), [&result](std::pair<Base, double> el){
        result.push_back(el.second);
    });

    const std::vector<double> goldenResult{{66.0, 132.0}};
    REQUIRE(std::equal(result.begin(), result.end(), goldenResult.begin(), goldenResult.end()));
}

#include "SystemWithDeletion.hpp"

TEST_CASE("Deletion")
{
    {
        SystemWithDeletion<Base> sys;
        auto prop = makeProperty<Base, double>(sys);
        auto en = sys.add();
        CHECK(!prop.empty());
        sys.erase(en);
        CHECK(prop.empty());
    }
    {
        SystemWithDeletion<Base> sys;
        auto prop = makeProperty<Base, double>(sys);
        auto en0 = sys.add();
        auto en1 = sys.add();
        auto en2 = sys.add();
        prop[en0] = 42.0;
        prop[en1] = 84.0;
        prop[en2] = 126.0;
        sys.erase(en1);
        CHECK(prop[en0] == 42.0);
        CHECK(prop[en2] == 126.0);
        CHECK(prop.size() == 2);
    }
}

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
    auto range = prop.asRange();
    REQUIRE(*begin(range) == 42.0);

    for_each(view::zip(sys.asRange(), prop.asRange()), [](std::pair<Base, double> el){
        std::cout << el.first.id() << ": " << el.second << std::endl;
    });
}

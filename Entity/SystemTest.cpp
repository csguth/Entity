#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "System.hpp"

using namespace Entity;

TEST_CASE("Empty")
{
    const System<Base> sys;
    CHECK(sys.empty());
    CHECK(sys.size() == 0);
}

TEST_CASE("Add")
{
    System<Base> sys;
    Base en;
    int called = 0;
    sys.connectOnAdd([&en, &called](Base en2)
    {
        en = en2;
        ++called;
    });
    CHECK(called == 0);
    const Base result = sys.add();
    CHECK(called == 1);
    CHECK(result == en);
    CHECK(sys.size() == 1);
    CHECK(!sys.empty());
    CHECK(sys.alive(result));
}

TEST_CASE("Reserve")
{
    System<Base> sys;
    CHECK(sys.capacity() == 0);
    System<Base>::size_type size = 0;
    sys.connectOnReserve([&size](System<Base>::size_type newSize)
    {
        size = newSize;
    });
    CHECK(size == 0);
    sys.reserve(42);
    CHECK(sys.capacity() == 42);
    CHECK(size == 42);
}

#include <boost/serialization/strong_typedef.hpp>

BOOST_STRONG_TYPEDEF(Base, Custom)

TEST_CASE("Templated")
{
    System<Custom> sys;
    System<Base>   sys2;
    const auto entity = sys.add();
    CHECK(sys.alive(entity));
    const auto entity2 = sys2.add();
    CHECK(typeid(entity2) != typeid(entity));
}

namespace
{
struct OnAddScopedConnectionSlot
{
    static int counter;
    boost::signals2::scoped_connection connection;
    void theSlot(Base) { ++counter; }
};
int OnAddScopedConnectionSlot::counter;
}

TEST_CASE("Scoped Signal")
{
    System<Base> sys;
    {
        OnAddScopedConnectionSlot slot{sys.connectOnAdd(std::bind(&OnAddScopedConnectionSlot::theSlot, &slot, std::placeholders::_1))};
        sys.add();
        CHECK(OnAddScopedConnectionSlot::counter == 1);
    }
    sys.add();
    CHECK(OnAddScopedConnectionSlot::counter == 1);
}

using namespace ranges;
#include <iostream>
TEST_CASE("As Range")
{
    System<Base> sys;
    const std::array<Base, 3> entities{{sys.add(), sys.add(), sys.add()}};
    CHECK(sys.asRange().size() == entities.size());
    auto range   = sys.asRange();
    auto lastTwo = sys.asRange() | view::drop(1);
    auto last    = sys.asRange() | view::drop(2);
    CHECK(entities[0] == *begin(range));
    CHECK(entities[1] == *begin(lastTwo));
    CHECK(entities[2] == *begin(last));
}

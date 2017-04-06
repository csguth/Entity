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
    std::size_t size = 0;
    sys.connectOnReserve([&size](std::size_t newSize)
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

#include "SystemWithDeletion.hpp"

TEST_CASE("Deletion")
{
    {
        SystemWithDeletion<Base> sys;
        const Base b = sys.add();
        sys.erase(b);
        CHECK(sys.empty());
        CHECK(!sys.alive(b));
    }
    {
        SystemWithDeletion<Base> sys;
        sys.connectOnAdd([](Base)
        {

        });
        const Base en0 = sys.add();
        const Base en1 = sys.add();
        sys.erase(en0);
        CHECK(!sys.alive(en0));
        CHECK(sys.alive(en1));
    }
    {
        SystemWithDeletion<Base> sys;
        std::vector<Base> erased;
        sys.connectOnErase([&erased](const Base en)
        {
            erased.push_back(en);
        });
        const Base en0 = sys.add();
        const Base en1 = sys.add();
        const Base en2 = sys.add();
        sys.erase(en0);
        sys.erase(en2);
        const std::vector<Base> golden{{en0, en2}};
        CHECK(std::is_permutation(erased.begin(), erased.end(), golden.begin(), golden.end()));
    }

}

TEST_CASE("Indexer")
{
    {
        System<Base> sys;
        auto indexer = sys.indexer();
        CHECK(indexer.use_count() == 2);
        auto en = sys.add();
        CHECK(indexer->lookup(en) == en.id());
        System<Base> sys2;
        auto indexer2 = sys2.indexer();
        CHECK(indexer2.get() == indexer.get());
    }
    {
        SystemWithDeletion<Base> sys;
        auto indexer = sys.indexer();
        CHECK(indexer.use_count() == 2);
        auto en = sys.add();
        auto en2 = sys.add();
        sys.erase(en);
        CHECK(indexer->lookup(en) == std::numeric_limits<std::size_t>::max());
        CHECK(indexer->lookup(en2) == 0);
        SystemWithDeletion<Base> sys2;
        auto indexer2 = sys2.indexer();
        CHECK(indexer2.get() != indexer.get());
    }
}

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "System.hpp"
#include "test.hpp"

using namespace Entity;

TEST_CASE("Empty")
{
    const System<Test::TestEntity> sys;
    CHECK(sys.empty());
    CHECK(sys.size() == 0);
    CHECK(!sys.alive(Test::TestEntity{}));
}

TEST_CASE("Add")
{
    System<Test::TestEntity> sys;
    Test::TestEntity en;
    int called = 0;
    sys.connectOnAdd([&en, &called](Test::TestEntity en2)
    {
        en = en2;
        ++called;
    });
    CHECK(called == 0);
    const Test::TestEntity result = sys.add();
    CHECK(called == 1);
    CHECK(result == en);
    CHECK(sys.size() == 1);
    CHECK(!sys.empty());
    CHECK(sys.alive(result));
}

TEST_CASE("Reserve")
{
    System<Test::TestEntity> sys;
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

namespace
{
struct OnAddScopedConnectionSlot
{
    static int counter;
    boost::signals2::scoped_connection connection;
    void theSlot(Test::TestEntity) { ++counter; }
};
int OnAddScopedConnectionSlot::counter;
}

TEST_CASE("Scoped Signal")
{
    System<Test::TestEntity> sys;
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
    System<Test::TestEntity> sys;
    const std::array<Test::TestEntity, 3> entities{{sys.add(), sys.add(), sys.add()}};
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
        SystemWithDeletion<Test::TestEntity> sys;
        const Test::TestEntity b = sys.add();
        sys.erase(b);
        CHECK(sys.empty());
        CHECK(!sys.alive(b));
    }
    {
        SystemWithDeletion<Test::TestEntity> sys;
        sys.connectOnAdd([](Test::TestEntity)
        {

        });
        const Test::TestEntity en0 = sys.add();
        const Test::TestEntity en1 = sys.add();
        sys.erase(en0);
        CHECK(!sys.alive(en0));
        CHECK(sys.alive(en1));
    }
    {
        SystemWithDeletion<Test::TestEntity> sys;
        std::vector<Test::TestEntity> erased;
        sys.connectOnErase([&erased](const Test::TestEntity en)
        {
            erased.push_back(en);
        });
        const Test::TestEntity en0 = sys.add();
        const Test::TestEntity en1 = sys.add();
        const Test::TestEntity en2 = sys.add();
        sys.erase(en0);
        sys.erase(en2);
        const std::vector<Test::TestEntity> golden{{en0, en2}};
        CHECK(std::is_permutation(erased.begin(), erased.end(), golden.begin(), golden.end()));
    }

}

TEST_CASE("Indexer")
{
    {
        System<Test::TestEntity> sys;
        auto indexer = sys.indexer();
        CHECK(indexer.use_count() == 2);
        auto en = sys.add();
        CHECK(indexer->lookup(en) == en.id());
        System<Test::TestEntity> sys2;
        auto indexer2 = sys2.indexer();
        CHECK(indexer2.get() == indexer.get());
    }
    {
        SystemWithDeletion<Test::TestEntity> sys;
        auto indexer = sys.indexer();
        CHECK(indexer.use_count() == 2);
        auto en = sys.add();
        auto en2 = sys.add();
        sys.erase(en);
        CHECK(indexer->lookup(en) == std::numeric_limits<std::size_t>::max());
        CHECK(indexer->lookup(en2) == 0);
        SystemWithDeletion<Test::TestEntity> sys2;
        auto indexer2 = sys2.indexer();
        CHECK(indexer2.get() != indexer.get());
    }
}


#include "KeyWrapper.hpp"
TEST_CASE("Key Wrapper/ Empty")
{
    System<Test::TestEntity> sys;
    auto keyWrapper = makeKeyWrapper<std::string>(sys);
    CHECK(!keyWrapper.has("Entity"));
    REQUIRE_THROWS(keyWrapper.at("Entity"));
    CHECK(sys.size() == 0);
}

TEST_CASE("Key Wrapper/ Add")
{
    System<Test::TestEntity> sys;
    auto keyWrapper = makeKeyWrapper<std::string>(sys);
    auto entity = keyWrapper.addOrGet("Entity");
    CHECK(sys.size() == 1);
    CHECK(keyWrapper.key(entity) == "Entity");
    CHECK(keyWrapper.has("Entity"));
    CHECK(keyWrapper.at("Entity") == entity);
    auto entity2 = keyWrapper.addOrGet("Entity");
    CHECK(sys.size() == 1);
    CHECK(entity == entity2);
}

TEST_CASE("Key Wrapper/ Deletion")
{
   {
        SystemWithDeletion<Test::TestEntity> sys;
        auto keyWrapper = makeKeyWrapper<std::string>(sys);
        auto entity = keyWrapper.addOrGet("Entity");
        sys.erase(entity);
        CHECK(!keyWrapper.has("Entity"));
    }
    {
         SystemWithDeletion<Test::TestEntity> sys;
         auto keyWrapper = makeKeyWrapper<std::string>(sys);
         auto entity = keyWrapper.addOrGet("Entity");
         sys.erase(entity);
         auto entity2 = keyWrapper.addOrGet("Entity");
         CHECK(keyWrapper.has("Entity"));
         CHECK(entity != entity2);
    }
}

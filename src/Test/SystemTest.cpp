#include <catch.hpp>
#include <Entity/Core/System.hpp>
#include "test.hpp"

using namespace Entity;
using namespace ranges;

TEST_CASE_METHOD(Test::Fixture::Empty<System>, "empty", "[System]")
{
    CHECK(system.empty());
    CHECK(system.size() == 0);
    CHECK(system.capacity() == 0);
    CHECK(!system.alive(Test::TestEntity{}));
}

TEST_CASE_METHOD(Test::Fixture::WithOneEntity<System>, "add", "[System]")
{
    CHECK(system.size() == 1);
    CHECK(system.alive(entity[0]));
    CHECK(!system.empty());
}

TEST_CASE_METHOD(Test::Fixture::Empty<System>, "reserve", "[System]")
{
    system.reserve(42);
    CHECK(system.capacity() == 42);
}

TEST_CASE_METHOD(Test::Fixture::Empty<System>, "connectOnAdd", "[System]")
{
    Test::TestEntity en;
    int called = 0;
    system.notifier->onAdd.connect([&en, &called](Test::TestEntity en2)
    {
        en = en2;
        ++called;
    });
    CHECK(called == 0);
    auto result = system.add();
    CHECK(called == 1);
    CHECK(result == en);
    CHECK(system.size() == 1);
    CHECK(system.alive(result));
    CHECK(!system.empty());
}

TEST_CASE_METHOD(Test::Fixture::Empty<System>, "connectOnReserve", "[System]")
{
    auto callReserveAndReturnTheArg = [&](std::size_t value)
    {
        std::size_t size_;
        auto scoped = system.notifier->onReserve.connect([&](auto theSize)
        {
            size_ = theSize;
        });
        system.reserve(value);
        return size_;
    };
    CHECK(callReserveAndReturnTheArg(42) == 42);
    CHECK(callReserveAndReturnTheArg(666) == 666);
}

TEST_CASE_METHOD(Test::Fixture::WithThreeEntities<System>, "asRange", "[System]")
{
    auto range   = system.asRange();
    auto lastTwo = system.asRange() | view::drop(1);
    auto last    = system.asRange() | view::drop(2);
    CHECK(entity[0] == *begin(range));
    CHECK(entity[1] == *begin(lastTwo));
    CHECK(entity[2] == *begin(last));
}

TEST_CASE_METHOD(Test::Fixture::WithOneEntity<SystemWithDeletion>,"erase 1 of 1", "[System]")
{
    system.erase(entity[0]);
    CHECK(system.empty());
    CHECK(!system.alive(entity[0]));
}

TEST_CASE_METHOD(Test::Fixture::WithThreeEntitiesEraseFirst<SystemWithDeletion>,"erase 1 (first) of 3", "[System]")
{
    CHECK(system.size() == 2);
    CHECK(system.alive(entity[1]));
    CHECK(system.alive(entity[2]));
    CHECK(!system.alive(entity[0]));
}

TEST_CASE_METHOD(Test::Fixture::WithThreeEntities<SystemWithDeletion>,"erase 1 of 3", "[System]")
{
    system.erase(entity[1]);
    CHECK(system.size() == 2);
    CHECK(system.alive(entity[0]));
    CHECK(system.alive(entity[2]));
    CHECK(!system.alive(entity[1]));
}

TEST_CASE_METHOD(Test::Fixture::WithThreeEntities<SystemWithDeletion>,"erase 2 of 3", "[System]")
{
    system.erase(entity[1]);
    system.erase(entity[2]);
    CHECK(system.size() == 1);
    CHECK(system.alive(entity[0]));
    CHECK(!system.alive(entity[1]));
    CHECK(!system.alive(entity[2]));
}

TEST_CASE_METHOD(Test::Fixture::WithOneEntity<SystemWithDeletion>, "erase invalid", "[System]")
{
    CHECK_THROWS(system.erase(Test::TestEntity{}));
    CHECK(system.size() == 1);
    CHECK(system.alive(entity[0]));
    CHECK(!system.empty());
}

TEST_CASE_METHOD(Test::Fixture::WithThreeEntities<SystemWithDeletion>, "connectOnErase", "[System]")
{
    using ContainerType = std::vector<Test::TestEntity>;
    ContainerType erased;
    system.notifier->onErase.connect([&erased](auto en)
    {
        erased.push_back(en);
    });
    system.erase(entity[0]);
    system.erase(entity[2]);
    const ContainerType golden{{entity[0], entity[2]}};
    CHECK(std::equal(erased.begin(), erased.end(), golden.begin(), golden.end()));
}

TEST_CASE_METHOD(Test::Fixture::WithOneEntity<System>, "indexer", "[System]")
{
    auto indexer = system.indexer();
    CHECK(indexer.use_count() == 2);
    CHECK(indexer->lookup(entity[0]) == entity[0].id());
    System<Test::TestEntity> system2;
    auto indexer2 = system2.indexer();
    CHECK(indexer2.get() == indexer.get());
}

TEST_CASE_METHOD(Test::Fixture::WithThreeEntitiesEraseFirst<SystemWithDeletion>, "indexer (with deletion)", "[System]")
{
    auto indexer = system.indexer();
    CHECK(indexer->lookup(entity[0]) == Test::TestEntity().id());
    const std::vector<std::size_t> goldenIds{0, 1};
    const std::vector<std::size_t> ids = [&]()
    {
        std::vector<std::size_t> ids_;
        ids_ = (system.asRange() | view::transform([&](auto&& en) { return indexer->lookup(en); }));
        return ids_;
    }();
    CHECK(std::is_permutation(ids.begin(), ids.end(), goldenIds.begin(), goldenIds.end()));
    SystemWithDeletion<Test::TestEntity> system2;
    auto indexer2 = system2.indexer();
    CHECK(indexer2.get() != indexer.get());
}


TEST_CASE_METHOD(Test::Fixture::Empty<System>, "KeyWrapper empty", "[System]")
{
    auto keyWrapper = makeKeyWrapper<std::string>(system);
    CHECK(!keyWrapper.has(Test::Fixture::KeyWrapperWithEntity::key()));
    REQUIRE_THROWS(keyWrapper.at(Test::Fixture::KeyWrapperWithEntity::key()));
}

TEST_CASE_METHOD(Test::Fixture::KeyWrapperWithEntity, "KeyWrapper add", "[System]")
{
    CHECK(keyWrapper.key(entity) == key());
    CHECK(keyWrapper.has(key()));
    CHECK(keyWrapper.at(key()) == entity);
    CHECK(system.size() == 1);
}

TEST_CASE_METHOD(Test::Fixture::KeyWrapperWithEntity, "KeyWrapper get", "[System]")
{
    CHECK(keyWrapper.addOrGet(key()) == entity);
    CHECK(system.size() == 1);
}

TEST_CASE_METHOD(Test::Fixture::KeyWrapperWithEntity, "KeyWrapper erase", "[System]")
{
    system.erase(entity);
    CHECK(!keyWrapper.has(key()));
    auto entity2 = keyWrapper.addOrGet(key());
    CHECK(keyWrapper.has(key()));
    CHECK(entity != entity2);
}

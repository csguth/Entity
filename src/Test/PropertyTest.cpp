#include <algorithm>
#include "test.hpp"
#include <Entity/Core/Property.hpp>
#include <Entity/Core/SystemWithDeletion.hpp>
#include "test.hpp"

using namespace Entity;
TEST_CASE("Property", "[Property]")
{
    System<Test::TestEntity> sys;
    auto prop = makeProperty<double>(sys);
    CHECK(prop.empty());
    CHECK(prop.size() == sys.size());
}

TEST_CASE("Add After", "[Property]")
{
    System<Test::TestEntity> sys;
    auto prop = makeProperty<double>(sys);
    const Test::TestEntity en = sys.add();
    CHECK(prop.size() == sys.size());
    CHECK(prop.size() == 1);
}

TEST_CASE("Add Before", "[Property]")
{
    System<Test::TestEntity> sys;
    const Test::TestEntity en = sys.add();
    auto prop = makeProperty<double>(sys);
    CHECK(prop.size() == sys.size());
    CHECK(prop.size() == 1);
}

TEST_CASE("Capacity After", "[Property]")
{
    System<Test::TestEntity> sys;
    auto prop = makeProperty<double>(sys);
    sys.reserve(1024);
    CHECK(sys.capacity() == prop.capacity());
    CHECK(prop.capacity() == 1024);
}

TEST_CASE("Capacity Before", "[Property]")
{
    System<Test::TestEntity> sys;
    sys.reserve(1024);
    auto prop = makeProperty<double>(sys);
    CHECK(sys.capacity() == prop.capacity());
    CHECK(prop.capacity() == 1024);
}

TEST_CASE("Scoped Connections", "[Property]")
{
    System<Test::TestEntity> sys;
    {
        auto prop = makeProperty<double>(sys);
        sys.add();
        sys.add();
        sys.add();
    }
    CHECK_NOTHROW(sys.add());
}

using namespace ranges;
#include <iostream>
TEST_CASE("As Range", "[Property]")
{
    System<Test::TestEntity> sys;
    auto prop = makeProperty<double>(sys);
    prop[sys.add()] = 42.0;
    prop[sys.add()] = 84.0;
    auto range = prop.asRange();
    for_each(views::zip(sys.asRange(), prop.asRange()), [](std::pair<Test::TestEntity, double&> el){ // Should modify
        static double count = 1.0;
        el.second = 66.0 * count;
        count += 1.0;
    });

    for_each(views::zip(sys.asRange(), prop.asRange()), [](std::pair<Test::TestEntity, double> el){ // Should not modify
        static double count = 1.0;
        el.second = 77.0 * count;
        count += 1.0;
    });

    std::vector<double> result;
    for_each(views::zip(sys.asRange(), prop.asRange()), [&result](std::pair<Test::TestEntity, double> el){
        result.push_back(el.second);
    });

    const std::vector<double> goldenResult{{66.0, 132.0}};
    REQUIRE(std::equal(result.begin(), result.end(), goldenResult.begin(), goldenResult.end()));
}

TEST_CASE("Deletion", "[Property]")
{
    {
        SystemWithDeletion<Test::TestEntity> sys;
        auto prop = makeProperty<double>(sys);
        auto en = sys.add();
        CHECK(!prop.empty());
        sys.erase(en);
        CHECK(prop.empty());
    }
    {
        SystemWithDeletion<Test::TestEntity> sys;
        auto prop = makeProperty<double>(sys);
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

TEST_CASE("Independent Lifetimes", "[Property]")
{
    {
        Property<Test::TestEntity, double, System> prop;
        std::vector<Test::TestEntity> entities;
        {
            System<Test::TestEntity> sys;
            prop = makeProperty<double>(sys);
            entities.push_back(sys.add());
            entities.push_back(sys.add());
            entities.push_back(sys.add());
            prop[entities[0]] = 1.0;
            prop[entities[1]] = 2.0;
            prop[entities[2]] = 3.0;
        }
        CHECK(prop[entities[0]] == 1.0);
        CHECK(prop[entities[1]] == 2.0);
        CHECK(prop[entities[2]] == 3.0);
    }
    {
        Property<Test::TestEntity, double, SystemWithDeletion> prop;
        std::vector<Test::TestEntity> entities;
        {
            SystemWithDeletion<Test::TestEntity> sys;
            prop = makeProperty<double>(sys);
            entities.push_back(sys.add());
            entities.push_back(sys.add());
            entities.push_back(sys.add());
            prop[entities[0]] = 1.0;
            prop[entities[1]] = 2.0;
            prop[entities[2]] = 3.0;
        }
        CHECK(prop[entities[0]] == 1.0);
        CHECK(prop[entities[1]] == 2.0);
        CHECK(prop[entities[2]] == 3.0);
    }
}

TEST_CASE("Move", "[Property]")
{
    auto createProp = [](SystemWithDeletion<Test::TestEntity>& theSys)
    {
        return makeProperty<double>(theSys);
    };
    SystemWithDeletion<Test::TestEntity> sys;
    auto en0 = sys.add();
    auto en1 = sys.add();
    auto en2 = sys.add();

    // Move assignment
    Property<Test::TestEntity, double, SystemWithDeletion> prop;
    prop = createProp(sys);
    prop[en0] = 42.0;
    prop[en1] = 24.0;
    prop[en2] = 84.0;

    // Move construction
    Property<Test::TestEntity, double, SystemWithDeletion> prop2 = createProp(sys);
    CHECK(prop.size() == 3);

    // Copy assignment
    prop2 = prop;
    CHECK(prop2[en0] == 42.0);
    CHECK(prop2[en1] == 24.0);
    CHECK(prop2[en2] == 84.0);
    sys.add();
    CHECK(prop.size() == prop2.size());
    CHECK(prop.size() == 4);

    // Move assigment
    Property<Test::TestEntity, double, SystemWithDeletion> prop3;
    prop3 = std::move(prop2);
    CHECK(prop3[en0] == 42.0);
    CHECK(prop3[en1] == 24.0);
    CHECK(prop3[en2] == 84.0);
    sys.add();
    CHECK(prop.size() == prop3.size());
    CHECK(prop.size() == 5);

    // Copy construction
    Property<Test::TestEntity, double, SystemWithDeletion> prop4(prop3);
    CHECK(prop4[en0] == 42.0);
    CHECK(prop4[en1] == 24.0);
    CHECK(prop4[en2] == 84.0);
}

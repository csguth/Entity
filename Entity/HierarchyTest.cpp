#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "HierarchyTest.hpp"
#include "SystemWithDeletion.hpp"
#include "Composition.hpp"

using namespace Entity;

TEST_CASE("Composition 1-N")
{
    {
        auto parentSystem = System<Test::Parent>{};
        auto childSystem  = System<Test::Child>{};
        auto composition = makeComposition<Left>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        auto child2  = childSystem.add();
        composition.firstChild(parent, child);
        composition.nextSibling(child, child2);
        CHECK(composition.firstChild(parent) == child);
        CHECK(composition.nextSibling(child) == child2);
    }
    {
        auto parentSystem = System<Test::Parent>{};
        auto childSystem  = System<Test::Child>{};
        auto composition = makeComposition<Right>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto parent2 = parentSystem.add();
        auto child  = childSystem.add();
        auto child2 = childSystem.add();
        composition.parent(child, parent);
        composition.parent(child2, parent2);
        CHECK(composition.parent(child) == parent);
        CHECK(composition.parent(child2) == parent2);
        CHECK(!(composition.parent(child) == composition.parent(child2)));
    }
    {
        auto parentSystem = System<Test::Parent>{};
        auto childSystem  = System<Test::Child>{};
        auto composition = makeComposition<Both>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        auto child2  = childSystem.add();
        composition.firstChild(parent, child);
        composition.nextSibling(child, child2);
        composition.parent(child, parent);
        composition.parent(child2, parent);
        CHECK(composition.firstChild(parent) == child);
        CHECK(composition.nextSibling(child) == child2);
        CHECK(composition.parent(child) == parent);
        CHECK(composition.parent(child2) == parent);
    }

}

TEST_CASE("Add child")
{
    {
        auto parentSystem = System<Test::Parent>{};
        auto childSystem  = System<Test::Child>{};
        auto composition = makeComposition<Left>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        auto child2  = childSystem.add();
        composition.addChild(parent, child);
        composition.addChild(parent, child2);
        CHECK(composition.firstChild(parent) == child2);
        CHECK(composition.nextSibling(child2) == child);
    }
    {
        auto parentSystem = System<Test::Parent>{};
        auto childSystem  = System<Test::Child>{};
        auto composition = makeComposition<Right>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto parent2 = parentSystem.add();
        auto child  = childSystem.add();
        auto child2 = childSystem.add();
        composition.parent(child, parent);
        composition.parent(child2, parent2);
        composition.addChild(parent, child);
        composition.addChild(parent2, child2);
        CHECK(composition.parent(child) == parent);
        CHECK(composition.parent(child2) == parent2);
    }
    {
        auto parentSystem = System<Test::Parent>{};
        auto childSystem  = System<Test::Child>{};
        auto composition = makeComposition<Both>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        auto child2  = childSystem.add();
        composition.firstChild(parent, child);
        composition.nextSibling(child, child2);
        composition.parent(child, parent);
        composition.parent(child2, parent);
        composition.addChild(parent, child);
        composition.addChild(parent, child2);
        CHECK(composition.parent(child) == parent);
        CHECK(composition.parent(child2) == parent);
        CHECK(composition.firstChild(parent) == child2);
        CHECK(composition.nextSibling(child2) == child);
    }

}

TEST_CASE("Range adaptors")
{
    auto parentSystem = System<Test::Parent>{};
    auto childSystem  = System<Test::Child>{};
    auto parent0 = parentSystem.add();
    auto parent1 = parentSystem.add();
    LeftMapped<Test::Parent, System, Test::Child, System> leftMapped(parentSystem, childSystem);
    leftMapped.addChild(parent0, childSystem.add());
    leftMapped.addChild(parent0, childSystem.add());
    leftMapped.addChild(parent0, childSystem.add());
    leftMapped.addChild(parent1, childSystem.add());
    ranges::for_each(leftMapped.children(parent0), [](Test::Child child)
    {
        std::cout << static_cast<Base>(child).id() << std::endl;
    });
}




#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "HierarchyTest.hpp"
#include "SystemWithDeletion.hpp"
#include "Composition.hpp"

using namespace Entity;

TEST_CASE("Composition 1-N")
{
    {
        auto parentSystem = SystemWithDeletion<Test::Parent>{};
        auto childSystem  = SystemWithDeletion<Test::Child>{};
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
        auto parentSystem = SystemWithDeletion<Test::Parent>{};
        auto childSystem  = SystemWithDeletion<Test::Child>{};
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
        auto parentSystem = SystemWithDeletion<Test::Parent>{};
        auto childSystem  = SystemWithDeletion<Test::Child>{};
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
        auto parentSystem = SystemWithDeletion<Test::Parent>{};
        auto childSystem  = SystemWithDeletion<Test::Child>{};
        auto composition = makeComposition<Left>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        CHECK(composition.childrenSize(parent) == 0);
        auto child  = childSystem.add();
        auto child2  = childSystem.add();
        composition.addChild(parent, child);
        composition.addChild(parent, child2);
        CHECK(composition.firstChild(parent) == child2);
        CHECK(composition.nextSibling(child2) == child);
        CHECK(composition.childrenSize(parent) == 2);
    }
    {
        auto parentSystem = SystemWithDeletion<Test::Parent>{};
        auto childSystem  = SystemWithDeletion<Test::Child>{};
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
        auto parentSystem = SystemWithDeletion<Test::Parent>{};
        auto childSystem  = SystemWithDeletion<Test::Child>{};
        auto composition = makeComposition<Both>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        CHECK(composition.childrenSize(parent) == 0);
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
        CHECK(composition.childrenSize(parent) == 2);
    }

}

using namespace ranges::v3;
TEST_CASE("Range adaptors")
{
    auto parentSystem = SystemWithDeletion<Test::Parent>{};
    auto childSystem  = SystemWithDeletion<Test::Child>{};
    auto parent0 = parentSystem.add();
    auto parent1 = parentSystem.add();
    LeftMapped<Test::Parent, SystemWithDeletion, Test::Child, SystemWithDeletion> leftMapped(parentSystem, childSystem);
    Test::Child child00, child01, child02, child10;
    leftMapped.addChild(parent0, child00 = childSystem.add());
    leftMapped.addChild(parent0, child01 = childSystem.add());
    leftMapped.addChild(parent0, child02 = childSystem.add());
    leftMapped.addChild(parent1, child10 = childSystem.add());
    CHECK(ranges::count(leftMapped.children(parent0), child00) == 1);
    CHECK(ranges::count(leftMapped.children(parent0), child01) == 1);
    CHECK(ranges::count(leftMapped.children(parent0), child02) == 1);
    CHECK(ranges::count(leftMapped.children(parent0), Test::Child{}) == 0);
    CHECK(ranges::count(leftMapped.children(parent1), child10) == 1);
    CHECK(ranges::count(leftMapped.children(parent1), Test::Child{}) == 0);

//    auto rng0 = leftMapped.children(parent0);
//    auto rng = view::cartesian_product(rng0, rng0);
//    for_each(rng, [](auto pair)
//    {
//    });
}

TEST_CASE("Strong composition")
{
    {
        // Strong composition, erasing the Parent will erase the Children.
        // Left Mapped
        SystemWithDeletion<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Child> childSystem;
        auto composition = makeComposition<Left>(parentSystem, childSystem);

        const Test::Parent parent0 = parentSystem.add();
        const Test::Parent parent1 = parentSystem.add();
        const Test::Child child00 = childSystem.add();
        const Test::Child child01 = childSystem.add();
        const Test::Child child10 = childSystem.add();

        composition.addChild(parent0, child00);
        composition.addChild(parent0, child01);
        composition.addChild(parent1, child10);

        parentSystem.erase(parent0);

        CHECK(childSystem.size() == 1);
        CHECK(childSystem.alive(child10));
        CHECK(!childSystem.alive(child00));
        CHECK(!childSystem.alive(child01));

        CHECK(ranges::count(composition.children(parent1), child10) == 1);
    }

    {
        // Strong composition, erasing the Parent will erase the Children.
        // Both mapped
        SystemWithDeletion<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Child> childSystem;
        auto composition = makeComposition<Both>(parentSystem, childSystem);

        const Test::Parent parent0 = parentSystem.add();
        const Test::Parent parent1 = parentSystem.add();
        const Test::Child child00 = childSystem.add();
        const Test::Child child01 = childSystem.add();
        const Test::Child child10 = childSystem.add();

        composition.addChild(parent0, child00);
        composition.addChild(parent0, child01);
        composition.addChild(parent1, child10);

        parentSystem.erase(parent0);

        CHECK(childSystem.size() == 1);
        CHECK(childSystem.alive(child10));
        CHECK(!childSystem.alive(child00));
        CHECK(!childSystem.alive(child01));

        CHECK(composition.parent(child10) == parent1);
        CHECK(ranges::count(composition.children(parent1), child10) == 1);
    }

    {
        // Weak composition, we can't really erase Parent entity.
        System<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Child> childSystem;
        auto composition = makeComposition<Right>(parentSystem, childSystem);
    }

    {
        // Weak composition, we can't really erase either Parent or Child entities.
        System<Test::Parent> parentSystem;
        System<Test::Child> childSystem;
        auto composition = makeComposition<Left>(parentSystem, childSystem);
    }
    {
        // Weak composition, we can't really erase Child entity.
        SystemWithDeletion<Test::Parent> parentSystem;
        System<Test::Child> childSystem;
        auto composition = makeComposition<Left>(parentSystem, childSystem);
    }
    {
        // Weak composition, all the Right mapped compositions are weak.
        SystemWithDeletion<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Parent> childSystem;
        auto composition = makeComposition<Right>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        composition.addChild(parent, child);
        parentSystem.erase(parent);
        CHECK(childSystem.alive(child));
        CHECK(!parentSystem.alive(composition.parent(child)));
    }

}

TEST_CASE("Weak adapter")
{
    {
        // With the weak adapter
        SystemWithDeletion<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Child> childSystem;
        WeakAdapter<Test::Child> adapter(childSystem); // This adapter prevents the child of being erased.
        auto composition = makeComposition<Left>(parentSystem, adapter);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        composition.addChild(parent, child);
        CHECK(composition.childrenSize(parent) == 1);
        parentSystem.erase(parent);
        CHECK(childSystem.alive(child));
        parent = parentSystem.add();
        composition.addChild(parent, child);
        CHECK(composition.childrenSize(parent) == 1);
        childSystem.erase(child);
        // WARNING: Since the children doesn't map to its parent, the parent->child ref is now out-of-sync.
        CHECK(composition.childrenSize(parent) == 1);
    }
    {
        // Without the weak adapter
        SystemWithDeletion<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Child> childSystem;
        auto composition = makeComposition<Left>(parentSystem, childSystem);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        composition.addChild(parent, child);
        parentSystem.erase(parent);
        CHECK(!childSystem.alive(child));
    }
    {
        // With the weak adapter for both mapped
        SystemWithDeletion<Test::Parent> parentSystem;
        SystemWithDeletion<Test::Child> childSystem;
        WeakAdapter<Test::Child> adapter(childSystem);
        auto composition = makeComposition<Both>(parentSystem, adapter);
        auto parent = parentSystem.add();
        auto child  = childSystem.add();
        auto child2 = childSystem.add();
        composition.addChild(parent, child);
        composition.addChild(parent, child2);
        CHECK(composition.childrenSize(parent) == 2);
        parentSystem.erase(parent);
        CHECK(childSystem.alive(child));
        CHECK(childSystem.alive(child2));
        CHECK(composition.parent(child) == Test::Parent{}); // Children parents are sync'd.
        CHECK(composition.parent(child2) == Test::Parent{});
        parent = parentSystem.add();
        composition.addChild(parent, child);
        composition.addChild(parent, child2);
        CHECK(composition.parent(child) == parent);
        CHECK(composition.parent(child2) == parent);
        childSystem.erase(child);
        CHECK(composition.childrenSize(parent) == 1);
        CHECK(count(composition.children(parent), child) == 0);
        CHECK(count(composition.children(parent), child2) == 1);
    }
}



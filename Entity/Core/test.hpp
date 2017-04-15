#ifndef TEST_HPP
#define TEST_HPP

#include "System.hpp"

namespace Test
{

struct TestEntity: Entity::Base<TestEntity>
{
    using Entity::Base<TestEntity>::Base;
    static std::string name()
    {
        return "TestEntity";
    }
};

}

#endif // TEST_HPP

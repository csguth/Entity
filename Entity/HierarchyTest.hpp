#ifndef HIERARCHYTEST_HPP
#define HIERARCHYTEST_HPP

#include "System.hpp"

namespace Test
{
    struct Parent: Entity::Base<Parent>
    {
        using Entity::Base<Parent>::Base;
        static std::string name()
        {
            return "Parent";
        }
    };
    struct Child: Entity::Base<Child>
    {
        using Entity::Base<Child>::Base;
        static std::string name()
        {
            return "Child";
        }
    };
}

#endif // HIERARCHYTEST_HPP

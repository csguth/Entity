#ifndef HIERARCHYTEST_HPP
#define HIERARCHYTEST_HPP
#include <boost/serialization/strong_typedef.hpp>

#include "System.hpp"

namespace Test
{
    BOOST_STRONG_TYPEDEF(Entity::Base, Parent)
    BOOST_STRONG_TYPEDEF(Entity::Base, Child)
}

#endif // HIERARCHYTEST_HPP

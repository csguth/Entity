#ifndef TEST_HPP
#define TEST_HPP

#include <catch2/catch.hpp>

#include <Entity/Core/KeyWrapper.hpp>
#include <Entity/Core/System.hpp>
#include <Entity/Core/SystemWithDeletion.hpp>

namespace Test
{

ENTITY_ENTITY_DECLARATION(TestEntity)

namespace Fixture
{

template <template <typename> class SystemType>
struct Empty
{
    SystemType<TestEntity> system;
};

template <std::size_t K, template <typename> class SystemType>
struct WithKEntities : Empty<SystemType>
{
    std::array<TestEntity, K> entity;
    WithKEntities() :
        Empty<SystemType>()
    {
        for(auto& en : entity)
        {
            en = this->system.add();
        }
    }
};

template <template <typename> class SystemType>
using WithOneEntity = WithKEntities<1, SystemType>;

template <template <typename> class SystemType>
using WithThreeEntities = WithKEntities<3, SystemType>;

template <template <typename> class SystemType>
struct WithThreeEntitiesEraseFirst : WithThreeEntities<SystemType>
{
    WithThreeEntitiesEraseFirst() :
        WithThreeEntities<SystemType>()
    {
        this->system.erase(this->entity[0]);
    }
};

struct KeyWrapperWithEntity : Empty<Entity::SystemWithDeletion>
{
    using KeyWrapperType = decltype(Entity::makeKeyWrapper<std::string>(system));
    KeyWrapperWithEntity() :
        Empty<Entity::SystemWithDeletion>(),
        keyWrapper(Entity::makeKeyWrapper<std::string>(system)),
        entity(keyWrapper.addOrGet(key()))
    {

    }

    KeyWrapperType keyWrapper;
    TestEntity entity;
    static const std::string key()
    {
        return "13df75b2-399f-11e7-a919-92ebcb67fe33";
    }
};

}

}

#endif // TEST_HPP

#ifndef KEYWRAPPER_HPP
#define KEYWRAPPER_HPP

#include <functional>
#include <unordered_map>
#include "Property.hpp"

namespace Entity
{

template <typename ValueType, typename KeyType, template <typename> class SystemType>
class KeyWrapper final
{
public:
    KeyWrapper(SystemType<ValueType>& system) :
        m_system{ system },
        keys{ makeProperty<KeyType>(system) }
    {

    }

    ValueType addOrGet(const KeyType& key)
    {
        if (!has(key))
        {
            ValueType en = m_system.get().add();
            values[key] = en;
            keys[en] = key;
            return en;
        }
        return values[key];
    }

    bool has(const KeyType& key) const
    {
        auto resultIt = values.find(key);
        if (resultIt != values.end())
        {
            if (m_system.get().alive(resultIt->second))
            {
                return true;
            }
            values.erase(resultIt);
            return false;
        }
        return false;
    }


private:
    std::reference_wrapper<SystemType<ValueType>> m_system;
public:
    Property<ValueType, KeyType, SystemType> keys;
    mutable std::unordered_map<KeyType, ValueType> values;

};

template <typename KeyType, typename ValueType, template <typename> class SystemType>
KeyWrapper<ValueType, KeyType, SystemType> makeKeyWrapper(SystemType<ValueType>& system)
{
    return {system};
}

}

#endif // KEYWRAPPER_HPP

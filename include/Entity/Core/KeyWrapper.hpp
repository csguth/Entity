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
        m_system(system),
        m_keys(makeProperty<KeyType>(system))
    {

    }
    ~KeyWrapper()
    {

    }

    ValueType addOrGet(KeyType key)
    {
        if (!has(key))
        {
            ValueType en = m_system.get().add();
            m_map[key] = en;
            m_keys[en] = key;
            return en;
        }
        return at(key);
    }

    bool has(KeyType key) const
    {
        auto resultIt = m_map.find(key);
        if (resultIt != m_map.end())
        {
            if (m_system.get().alive(resultIt->second))
            {
                return true;
            }
            m_map.erase(resultIt);
            return false;
        }
        return false;
    }

    ValueType at(KeyType key) const
    {
        return m_map.at(key);
    }

    const KeyType& key(ValueType en) const
    {
        return m_keys[en];
    }

private:
    std::reference_wrapper<SystemType<ValueType>> m_system;
    Property<ValueType, KeyType, SystemType> m_keys;
    mutable std::unordered_map<KeyType, ValueType> m_map;

};

template <typename KeyType, typename ValueType, template <typename> class SystemType>
KeyWrapper<ValueType, KeyType, SystemType> makeKeyWrapper(SystemType<ValueType>& system)
{
    return {system};
}

}

#endif // KEYWRAPPER_HPP

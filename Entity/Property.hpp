#ifndef PROPERTY_H
#define PROPERTY_H

#include "System.hpp"

namespace Entity
{

template <class KeyType, class ValueType>
class Property
{
public:
    Property()
    {

    }
    Property(System<KeyType>& sys):
        Property()
    {
        m_values.reserve(sys.capacity());
        m_values.resize(sys.size());
        m_onAddConnection     = std::move(sys.connectOnAdd(std::bind(&Property<KeyType, ValueType>::onAdd, this, std::placeholders::_1)));
        m_onReserveConnection = std::move(sys.connectOnReserve(std::bind(&Property<KeyType, ValueType>::onReserve, this, std::placeholders::_1)));
    }
    constexpr typename std::vector<ValueType>::size_type size() const
    {
        return m_values.size();
    }
    constexpr bool empty() const
    {
        return m_values.empty();
    }
    constexpr typename std::vector<ValueType>::size_type capacity() const
    {
        return m_values.capacity();
    }
    typename std::vector<ValueType>::reference operator[](KeyType key)
    {
        return m_values[key.id()];
    }
    typename std::vector<ValueType>::const_reference operator[](KeyType key) const
    {
        return m_values[key.id()];
    }
protected:
    void onAdd(KeyType)
    {
        m_values.push_back(ValueType{});
    }
    void onReserve(typename System<KeyType>::size_type size)
    {
        m_values.reserve(size);
    }
private:
    std::vector<ValueType> m_values;
    boost::signals2::scoped_connection m_onAddConnection;
    boost::signals2::scoped_connection m_onReserveConnection;
};

template <class KeyType, class ValueType>
Property<KeyType, ValueType> makeProperty(System<KeyType>& system)
{
    return Property<KeyType, ValueType>(system);
}

}

#endif // PROPERTY_H

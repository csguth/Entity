#ifndef PROPERTY_H
#define PROPERTY_H

#include "System.hpp"
#include <iostream>
#include <type_traits>
namespace Entity
{


template<class T, class Callable>
auto connectOnErase(int, T& sys, boost::signals2::scoped_connection& connection, Callable cb) -> decltype(&T::erase, void())
{
    connection = std::move(sys.connectOnErase(cb));
}

template<class T, class Callable>
auto connectOnErase(char, T&, boost::signals2::scoped_connection&, Callable) -> decltype(void(), void())
{
}

template <typename KeyType, typename ValueType, template <typename> class SystemType>
class Property
{
public:
    Property():
        m_system(nullptr)
    {

    }
    Property(SystemType<KeyType>& sys):
        m_system(&sys)
    {
        m_values.reserve(sys.capacity());
        m_values.resize(sys.size());
        m_onAddConnection     = std::move(sys.connectOnAdd([this](KeyType en)
        {
            this->onAdd(en);
        }));
        m_onReserveConnection = std::move(sys.connectOnReserve([this](std::size_t size)
        {
            this->onReserve(size);
        }));
        connectOnErase(0, sys, m_onEraseConnection, [this](KeyType en)
        {
            this->onErase(en);
        });
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
        return m_values[m_system->lookup(key)];
    }
    typename std::vector<ValueType>::const_reference operator[](KeyType key) const
    {
        return m_values[m_system->lookup(key)];
    }
    auto asRange()
    {
        return ranges::make_iterator_range(m_values.begin(), m_values.end());
    }
protected:
    void onAdd(KeyType)
    {
        m_values.push_back(ValueType{});
    }
    void onReserve(std::size_t size)
    {
        m_values.reserve(size);
    }
    void onErase(KeyType en)
    {
        std::swap(m_values.back(), m_values[m_system->lookup(en)]);
        m_values.pop_back();
    }

private:
    const SystemType<KeyType>*         m_system;
    std::vector<ValueType>             m_values;
    boost::signals2::scoped_connection m_onAddConnection;
    boost::signals2::scoped_connection m_onReserveConnection;
    boost::signals2::scoped_connection m_onEraseConnection;
};

template <typename ValueType, typename KeyType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType> makeProperty(SystemType<KeyType>& system)
{
    return {system};
}

}

#endif // PROPERTY_H

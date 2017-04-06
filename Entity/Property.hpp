#ifndef PROPERTY_H
#define PROPERTY_H

#include "System.hpp"
#include <iostream>
#include <type_traits>
namespace Entity
{

template <typename KeyType, typename ValueType, template <typename> class SystemType>
class Property final
{
public:
    Property()
    {

    }
    ~Property() = default;
    Property(Property&& other):
        Property()
    {
        using std::swap;
        swap(*this, other);
    }
    Property& operator=(Property&& other)
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }
    Property(const Property& other):
        m_indexer(other.m_indexer),
        m_notifier(other.m_notifier),
        m_values(other.m_values)
    {
        connectSignals();
    }
    Property& operator=(const Property& other)
    {
        Property copy(other);
        return *this = std::move(copy);
    }
    Property(SystemType<KeyType>& sys):
        m_indexer(sys.indexer()),
        m_notifier(sys.notifier())
    {
        m_values.reserve(sys.capacity());
        m_values.resize(sys.size());
        connectSignals();
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
        return m_values[m_indexer->lookup(key)];
    }
    typename std::vector<ValueType>::const_reference operator[](KeyType key) const
    {
        return m_values[m_indexer->lookup(key)];
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
        std::swap(m_values.back(), m_values[m_indexer->lookup(en)]);
        m_values.pop_back();
    }
    void connectSignals()
    {
        if(auto notifier = m_notifier.lock())
        {
            m_onAddConnection     = std::move(notifier->connectOnAdd([this](KeyType en)
            {
                this->onAdd(en);
            }));
            m_onReserveConnection = std::move(notifier->connectOnReserve([this](std::size_t size)
            {
                this->onReserve(size);
            }));
            m_onEraseConnection   = std::move(notifier->connectOnErase([this](KeyType en)
            {
                this->onErase(en);
            }));
        }
    }
    friend void swap(Property& first, Property& second)
    {
        using std::swap;
        swap(first.m_indexer,   second.m_indexer);
        swap(first.m_notifier, second.m_notifier);
        swap(first.m_values,   second.m_values);
        first.connectSignals();
    }

private:
    std::shared_ptr<typename SystemType<KeyType>::Indexer> m_indexer;
    std::weak_ptr<typename SystemType<KeyType>::Notifier>  m_notifier;
    std::vector<ValueType>                                 m_values;
    boost::signals2::scoped_connection                     m_onAddConnection;
    boost::signals2::scoped_connection                     m_onReserveConnection;
    boost::signals2::scoped_connection                     m_onEraseConnection;
};

template <typename ValueType, typename KeyType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType> makeProperty(SystemType<KeyType>& system)
{
    return {system};
}

}

#endif // PROPERTY_H

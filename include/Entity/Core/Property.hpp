#ifndef PROPERTY_H
#define PROPERTY_H

#include <Entity/Core/System.hpp>

namespace Entity
{

template <typename KeyType, typename ValueType, template <typename> class SystemType>
class Property final
{
public:
    Property();
    Property(Property&& other);
    Property(const Property& other);
    Property(SystemType<KeyType>& sys);
    ~Property() = default;
    Property& operator=(Property&& other);
    Property& operator=(const Property& other);
    constexpr typename std::vector<ValueType>::size_type size() const;
    constexpr bool empty() const;
    constexpr typename std::vector<ValueType>::size_type capacity() const;
    typename std::vector<ValueType>::reference operator[](KeyType key);
    typename std::vector<ValueType>::const_reference operator[](KeyType key) const;
    auto asRange();
    auto asRange() const;
    const ValueType* data() const;
    void disconnectOnErase();
    template <class RangeType>
    Property& operator=(RangeType range);
    friend void swap(Property& first, Property& second)
    {
        using std::swap;
        swap(first.m_indexer,   second.m_indexer);
        swap(first.m_notifier, second.m_notifier);
        swap(first.m_values,   second.m_values);
        first.connectSignals();
    }
    
private:
    void connectSignals();
    void onAdd(KeyType);
    void onReserve(std::size_t size);
public:
    void onErase(KeyType en);

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

#include "Property.ipp"

}

namespace ranges
{
inline namespace v3
{
namespace view
{
    template <typename ValueType, typename KeyType, template <typename> class SystemType>
    auto get(const Entity::Property<KeyType, ValueType, SystemType>& property)
    {
        return transform([&](KeyType key)
        {
            return property[key];
        });
    }

}
}
}

#endif // PROPERTY_H

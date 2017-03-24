#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <boost/signals2/signal.hpp>
#include <range/v3/all.hpp>

namespace Entity {

class Base
{
public:
    Base() : m_id(std::numeric_limits<uint32_t>::max())
    {

    }
    Base(uint32_t id) : m_id(id)
    {

    }
    virtual ~Base()
    {

    }
    Base(const Base& other) :
        m_id(other.m_id)
    {
    }
    bool operator==(const Base& other) const
    {
        return m_id == other.m_id;
    }
    uint32_t id() const
    {
        return m_id;
    }
    bool operator<(const Base& other) const
    {
        return m_id < other.m_id;
    }
private:
    uint32_t m_id;
};

template <class EntityType>
class System
{
public:
    using size_type       = typename std::vector<EntityType>::size_type;
    using OnAddSignal     = typename boost::signals2::signal<void(EntityType)> ;
    using OnReserveSignal = boost::signals2::signal<void(size_type)> ;

private:
    OnAddSignal m_onAdd;
    OnReserveSignal m_onReserve;

public:
    System()
    {

    }
    constexpr bool empty() const
    {
        return m_entities.empty();
    }
    constexpr size_type size() const
    {
        return m_entities.size();
    }
    constexpr size_type capacity() const
    {
        return m_entities.capacity();
    }
    EntityType add()
    {
        m_entities.emplace_back(m_entities.size());
        m_onAdd(m_entities.back());
        return m_entities.back();
    }
    boost::signals2::connection connectOnAdd(typename OnAddSignal::slot_type slot)
    {
        return m_onAdd.connect(std::move(slot));
    }
    boost::signals2::connection connectOnReserve(typename OnReserveSignal::slot_type slot)
    {
        return m_onReserve.connect(std::move(slot));
    }
    void reserve(size_type size)
    {
        m_onReserve(size);
        m_entities.reserve(size);
    }
    bool alive(EntityType entity) const
    {
        return static_cast<Base>(entity).id() < m_entities.size();
    }
    auto asRange() const
    {
        return ranges::make_iterator_range(m_entities.begin(), m_entities.end());
    }
private:
    std::vector<EntityType> m_entities;

};

}

#endif // SYSTEM_HPP

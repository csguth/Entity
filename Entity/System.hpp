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
class SystemBase
{
public:
    using OnAddSignal     = typename boost::signals2::signal<void(EntityType)>;
    using OnReserveSignal = boost::signals2::signal<void(std::size_t)>;

    virtual ~SystemBase()
    {

    }
    boost::signals2::connection connectOnAdd(typename OnAddSignal::slot_type slot)
    {
        return m_onAdd.connect(std::move(slot));
    }
    boost::signals2::connection connectOnReserve(typename OnReserveSignal::slot_type slot)
    {
        return m_onReserve.connect(std::move(slot));
    }
    constexpr bool empty() const
    {
        return m_entities.empty();
    }
    constexpr std::size_t size() const
    {
        return m_entities.size();
    }
    constexpr std::size_t capacity() const
    {
        return m_entities.capacity();
    }
    auto asRange() const
    {
        return ranges::make_iterator_range(m_entities.begin(), m_entities.end());
    }

protected:
    std::vector<EntityType> m_entities;
    OnAddSignal             m_onAdd;
    OnReserveSignal         m_onReserve;

};

template <class EntityType>
class System: public SystemBase<EntityType>
{
public:
    using typename SystemBase<EntityType>::OnAddSignal;
    using typename SystemBase<EntityType>::OnReserveSignal;

    System()
    {

    }
    EntityType add()
    {
        this->m_entities.emplace_back(this->size());
        this->m_onAdd(this->m_entities.back());
        return this->m_entities.back();
    }
    void reserve(std::size_t size)
    {
        this->m_onReserve(size);
        this->m_entities.reserve(size);
    }
    bool alive(EntityType entity) const
    {
        return static_cast<Base>(entity).id() < this->m_entities.size();
    }
    std::size_t lookup(EntityType en) const
    {
        return static_cast<Base>(en).id();
    }
private:
};

}

#endif // SYSTEM_HPP


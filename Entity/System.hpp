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
    class Notifier final
    {
    public:
        friend SystemBase;
        using OnAddSignal     = typename boost::signals2::signal<void(EntityType)>;
        using OnReserveSignal = boost::signals2::signal<void(std::size_t)>;
        using OnEraseSignal   = typename boost::signals2::signal<void(EntityType)>;

        ~Notifier() = default;

        boost::signals2::connection connectOnAdd(typename OnAddSignal::slot_type slot)
        {
            return m_onAdd.connect(std::move(slot));
        }
        boost::signals2::connection connectOnReserve(typename OnReserveSignal::slot_type slot)
        {
            return m_onReserve.connect(std::move(slot));
        }
        boost::signals2::connection connectOnErase(typename OnEraseSignal::slot_type slot)
        {
            return m_onErase.connect(std::move(slot));
        }
    private:
        OnAddSignal     m_onAdd;
        OnReserveSignal m_onReserve;
        OnEraseSignal   m_onErase;
    };

    SystemBase():
        m_notifier(std::make_shared<Notifier>())
    {

    }
    virtual ~SystemBase() = default;
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
    std::weak_ptr<Notifier> notifier() const
    {
        return {m_notifier};
    }
    boost::signals2::connection connectOnAdd(typename Notifier::OnAddSignal::slot_type slot)
    {
        return m_notifier->connectOnAdd(slot);
    }
    boost::signals2::connection connectOnReserve(typename Notifier::OnReserveSignal::slot_type slot)
    {
        return m_notifier->connectOnReserve(slot);
    }
    boost::signals2::connection connectOnErase(typename Notifier::OnEraseSignal::slot_type slot)
    {
        return m_notifier->connectOnErase(slot);
    }

protected:
    void notifyAdd()
    {
        m_notifier->m_onAdd(m_entities.back());
    }
    void notifyReserve(std::size_t size)
    {
        m_notifier->m_onReserve(size);
    }
    void notifyErase(EntityType en)
    {
        m_notifier->m_onErase(en);
    }

    std::vector<EntityType>   m_entities;
    std::shared_ptr<Notifier> m_notifier;
};

template <class EntityType>
class System: public SystemBase<EntityType>
{
public:
    struct Indexer
    {
        std::size_t lookup(EntityType en) const
        {
            return static_cast<Base>(en).id();
        }
    };

    System()
    {

    }
    EntityType add()
    {
        this->m_entities.emplace_back(this->size());
        this->notifyAdd();
        return this->m_entities.back();
    }
    void reserve(std::size_t size)
    {
        this->notifyReserve(size);
        this->m_entities.reserve(size);
    }
    bool alive(EntityType entity) const
    {
        return static_cast<Base>(entity).id() < this->m_entities.size();
    }
    std::shared_ptr<Indexer> indexer() const
    {
        static std::shared_ptr<Indexer> staticIndexer{std::make_shared<Indexer>()};
        return staticIndexer;
    }
private:
};

}

#endif // SYSTEM_HPP


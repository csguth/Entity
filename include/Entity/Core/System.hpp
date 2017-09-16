#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/signal_type.hpp>
#include <range/v3/all.hpp>

namespace Entity {

template<class Derived>
class Base
{
public:
    Base() : m_id(std::numeric_limits<std::size_t>::max())
    {

    }
    Base(std::size_t id) : m_id(id)
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
    bool operator!=(const Base& other) const
    {
        return m_id != other.m_id;
    }
    std::size_t id() const
    {
        return m_id;
    }
    bool operator<(const Base& other) const
    {
        return m_id < other.m_id;
    }
    friend std::ostream& operator<<(std::ostream& out, const Base<Derived>& this_)
    {
        return out << Derived::name() << "(" << this_.m_id << ")";
    }
private:
    std::size_t m_id;
};
}

#define ENTITY_ENTITY_DECLARATION(__name) \
struct __name : Entity::Base<__name>\
{\
    using Base<__name>::Base;\
    static std::string name()\
    {\
        return #__name;\
    }\
};

namespace Entity
{

using mutex_type = boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>;

template <template <typename> class BaseType, class EntityType>
class SystemBase
{
public:
    class Notifier final
    {
    public:
        friend SystemBase;
        using OnAddSignal     = typename boost::signals2::signal_type<void(EntityType),  mutex_type>::type;
        using OnReserveSignal = typename boost::signals2::signal_type<void(std::size_t), mutex_type>::type;
        using OnEraseSignal   = typename boost::signals2::signal_type<void(EntityType),  mutex_type>::type;

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

    constexpr SystemBase():
        m_notifier(std::make_shared<Notifier>()),
        m_next(0)
    {

    }
    virtual ~SystemBase() = default;

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
    void reserve(std::size_t size)
    {
        m_notifier->m_onReserve(size);
        static_cast<BaseType<EntityType>*>(this)->doReserve(size);
    }
    EntityType add()
    {
        auto result = m_next;
        m_notifier->m_onAdd(result);
        static_cast<BaseType<EntityType>*>(this)->doAdd();
        m_next = EntityType(m_next.id() + 1);
        return result;
    }
    constexpr std::size_t capacity() const
    {
        return static_cast<const BaseType<EntityType>*>(this)->getCapacity();
    }
    constexpr std::size_t size() const
    {
       return static_cast<const BaseType<EntityType>*>(this)->getSize();
    }
    constexpr bool empty() const
    {
        return size() == 0;
    }
    bool alive(EntityType entity) const
    {
        return static_cast<const BaseType<EntityType>*>(this)->isAlive(entity);
    }
    auto indexer() const
    {
        return static_cast<const BaseType<EntityType>*>(this)->getIndexer();
    }
    auto asRange() const
    {
        return static_cast<const BaseType<EntityType>*>(this)->getRange();
    }

protected:
    void notifyErase(EntityType en)
    {
        m_notifier->m_onErase(en);
    }

    std::shared_ptr<Notifier> m_notifier;
    EntityType m_next;
};

template <class EntityType>
    class System: public SystemBase<::Entity::System, EntityType>
{
public:
    friend SystemBase<::Entity::System, EntityType>;
    struct Indexer
    {
        std::size_t lookup(EntityType en) const
        {
            return en.id();
        }
    };

    System();
    
protected:
    void doAdd() {}
    void doReserve(std::size_t capacity)
    {
        m_capacity = capacity;
    }
    constexpr std::size_t getSize() const
    {
        return getIndexer()->lookup(this->m_next);
    }
    bool isAlive(EntityType entity) const
    {
        return entity.id() < getSize();
    }
    std::shared_ptr<Indexer> getIndexer() const
    {
        static std::shared_ptr<Indexer> staticIndexer{std::make_shared<Indexer>()};
        return staticIndexer;
    }
    auto getRange() const
    {
        return ranges::view::iota(static_cast<std::size_t>(0), getSize()) | ranges::view::transform([](auto id){ return EntityType{id}; });
    }
    std::size_t getCapacity() const
    {
        return std::max(getSize(), m_capacity);
    }
private:
    std::size_t m_capacity;
};
    

template <class EntityType>
System<EntityType>::System() :
    SystemBase<::Entity::System, EntityType>::SystemBase(),
    m_capacity(0)
{
    
}

}

#endif // SYSTEM_HPP

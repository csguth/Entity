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
    Base();
    Base(std::size_t id);
    virtual ~Base();
    Base(const Base& other);
    bool operator==(const Base& other) const;
    bool operator!=(const Base& other) const;
    std::size_t id() const;
    bool operator<(const Base& other) const;
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
    class Notifier;
    
    constexpr SystemBase();
    virtual ~SystemBase() = default;
    void reserve(std::size_t size);
    EntityType add();
    constexpr std::size_t capacity() const;
    constexpr std::size_t size() const;
    constexpr bool empty() const;
    bool alive(EntityType entity) const;
    auto indexer() const;
    auto asRange() const;
    
    std::shared_ptr<Notifier> notifier;
    
protected:
    EntityType m_next;
    
};
    
template <template <typename> class BaseType, class EntityType>
class SystemBase<BaseType, EntityType>::Notifier final
{
public:
    friend SystemBase;
    using OnAddSignal     = typename boost::signals2::signal_type<void(EntityType),  mutex_type>::type;
    using OnReserveSignal = typename boost::signals2::signal_type<void(std::size_t), mutex_type>::type;
    using OnEraseSignal   = typename boost::signals2::signal_type<void(EntityType),  mutex_type>::type;
    
    ~Notifier() = default;
    
    OnAddSignal     onAdd;
    OnReserveSignal onReserve;
    OnEraseSignal   onErase;
    
};


template <class EntityType>
class System: public SystemBase<::Entity::System, EntityType>
{
public:
    friend SystemBase<::Entity::System, EntityType>;
    struct Indexer;
    
    System();
    
protected:
    void doAdd();
    void doReserve(std::size_t capacity);
    constexpr std::size_t getSize() const;
    bool isAlive(EntityType entity) const;
    std::shared_ptr<Indexer> getIndexer() const;
    auto getRange() const;
    std::size_t getCapacity() const;
    
private:
    std::size_t m_capacity;
    
};
    
template <class EntityType>
struct System<EntityType>::Indexer
{
    std::size_t lookup(EntityType en) const;
};

#include "System.ipp"
    
}

#endif // SYSTEM_HPP


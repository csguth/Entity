#ifndef SYSTEMWITHDELETION_HPP
#define SYSTEMWITHDELETION_HPP

#include "System.hpp"

namespace Entity
{

template <class EntityType>
class SystemWithDeletion: public SystemBase<::Entity::SystemWithDeletion, EntityType>
{
public:
    friend SystemBase<::Entity::SystemWithDeletion, EntityType>;
    class Indexer;

    SystemWithDeletion();
    void erase(EntityType entity);

protected:
    constexpr std::size_t getSize() const;
    auto getRange() const;
    void doReserve(std::size_t size);
    void doAdd();
    bool isAlive(EntityType entity) const;
    std::shared_ptr<Indexer> getIndexer() const;
    std::size_t getCapacity() const;

private:
    std::shared_ptr<Indexer> m_indexer;
    std::vector<EntityType>  m_entities;
};

template <class EntityType>
class SystemWithDeletion<EntityType>::Indexer
{
public:
    friend SystemWithDeletion;

    std::size_t lookup(EntityType en) const;

private:
    void put(EntityType en, std::size_t index);
    std::size_t allocate();

    std::vector<std::size_t> m_index;
};


#include "SystemWithDeletion.ipp"

}

#endif // SYSTEMWITHDELETION_HPP

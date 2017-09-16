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
    class Indexer
    {
    public:
        friend SystemWithDeletion;
        std::size_t lookup(EntityType en) const
        {
            return m_index.at(en.id());
        }
    private:
        void put(EntityType en, std::size_t index)
        {
            m_index[en.id()] = index;
        }
        std::size_t allocate()
        {
            m_index.push_back(std::numeric_limits<std::size_t>::max());
            return m_index.size() - 1;
        }

        std::vector<std::size_t> m_index;
    };

    SystemWithDeletion() :
        SystemBase<::Entity::SystemWithDeletion, EntityType>(),
        m_indexer(std::make_shared<Indexer>())
    {

    }
    void erase(EntityType entity)
    {
        this->notifyErase(entity);
        const std::size_t index = m_indexer->lookup(entity);
        EntityType& theEntity   = m_entities[index];
        EntityType& last        = m_entities.back();
        m_indexer->put(last, index);
        m_indexer->put(entity, std::numeric_limits<std::size_t>::max());
        std::swap(theEntity, last);
        m_entities.pop_back();
    }

protected:
    constexpr std::size_t getSize() const
    {
        return m_entities.size();
    }
    auto getRange() const
    {
        return ranges::make_iterator_range(m_entities.begin(), m_entities.end());
    }
    void doReserve(std::size_t size)
    {
        m_entities.reserve(size);
    }
    void doAdd()
    {
        auto newIndex = m_indexer->allocate();
        m_entities.emplace_back(newIndex);
        m_indexer->put(m_entities.back(), m_entities.size()-1);
    }
    bool isAlive(EntityType entity) const
    {
        return entity != EntityType{} && m_indexer->lookup(entity) != std::numeric_limits<std::size_t>::max();
    }
    std::shared_ptr<Indexer> getIndexer() const
    {
        return m_indexer;
    }
    std::size_t getCapacity() const
    {
        return m_entities.capacity();
    }

private:
    std::shared_ptr<Indexer> m_indexer;
    std::vector<EntityType>  m_entities;
};

}

#endif // SYSTEMWITHDELETION_HPP

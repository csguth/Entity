#ifndef SYSTEMWITHDELETION_HPP
#define SYSTEMWITHDELETION_HPP

#include "System.hpp"

namespace Entity
{

template <class EntityType>
class SystemWithDeletion: public SystemBase<EntityType>
{
public:

    class Indexer
    {
    public:
        friend SystemWithDeletion;
        std::size_t lookup(EntityType en) const
        {
            return m_index[en.id()];
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

    SystemWithDeletion():
        m_indexer(std::make_shared<Indexer>())
    {

    }
    EntityType add()
    {
        auto newIndex = m_indexer->allocate();
        this->m_entities.emplace_back(newIndex);
        this->notifyAdd();
        m_indexer->put(this->m_entities.back(), this->m_entities.size()-1);
        return this->m_entities.back();
    }
    void erase(EntityType entity)
    {
        this->notifyErase(entity);
        const std::size_t index = m_indexer->lookup(entity);
        EntityType& theEntity   = this->m_entities[index];
        EntityType& last        = this->m_entities.back();
        m_indexer->put(last, index);
        m_indexer->put(entity, std::numeric_limits<std::size_t>::max());
        std::swap(theEntity, last);
        this->m_entities.pop_back();
    }
    bool alive(EntityType entity) const
    {
        return m_indexer->lookup(entity) != std::numeric_limits<std::size_t>::max();
    }
    std::shared_ptr<Indexer> indexer() const
    {
        return m_indexer;
    }
private:
    std::shared_ptr<Indexer> m_indexer;
};

}

#endif // SYSTEMWITHDELETION_HPP

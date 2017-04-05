#ifndef SYSTEMWITHDELETION_HPP
#define SYSTEMWITHDELETION_HPP

#include "System.hpp"

namespace Entity
{

template <class EntityType>
class SystemWithDeletion: public SystemBase<EntityType>
{
public:
    using OnEraseSignal = typename boost::signals2::signal<void(EntityType)>;

    boost::signals2::connection connectOnErase(typename OnEraseSignal::slot_type slot)
    {
        return m_onErase.connect(std::move(slot));
    }
    EntityType add()
    {
        this->m_entities.emplace_back(m_index.size());
        m_index.push_back(this->m_entities.size()-1);
        this->m_onAdd(this->m_entities.back());
        return this->m_entities.back();
    }
    void erase(EntityType entity)
    {
        m_onErase(entity);
        const std::size_t index = m_index[entity.id()];
        EntityType& theEntity   = this->m_entities[index];
        EntityType& last        = this->m_entities.back();
        m_index[last.id()]      = index;
        m_index[entity.id()]    = std::numeric_limits<std::size_t>::max();
        std::swap(theEntity, last);
        this->m_entities.pop_back();
    }
    bool alive(EntityType entity) const
    {
        return m_index.at(entity.id()) != std::numeric_limits<std::size_t>::max();
    }
    std::size_t lookup(EntityType en) const
    {
        return m_index[en.id()];
    }
private:
    std::vector<std::size_t> m_index;
    OnEraseSignal            m_onErase;
};

}

#endif // SYSTEMWITHDELETION_HPP

template <class EntityType>
std::size_t SystemWithDeletion<EntityType>::Indexer::lookup(EntityType en) const
{
    return m_index.at(en.id());
}
template <class EntityType>
void SystemWithDeletion<EntityType>::Indexer::put(EntityType en, std::size_t index)
{
    m_index[en.id()] = index;
}
template <class EntityType>
std::size_t SystemWithDeletion<EntityType>::Indexer::allocate()
{
    m_index.push_back(std::numeric_limits<std::size_t>::max());
    return m_index.size() - 1;
}
template <class EntityType>
SystemWithDeletion<EntityType>::SystemWithDeletion() :
    SystemBase<::Entity::SystemWithDeletion, EntityType>(),
    m_indexer(std::make_shared<Indexer>())
{

}
template <class EntityType>
void SystemWithDeletion<EntityType>::erase(EntityType entity)
{
    SystemBase<::Entity::SystemWithDeletion, EntityType>::notifier->onErase(entity);
    const std::size_t index = m_indexer->lookup(entity);
    EntityType& theEntity   = m_entities[index];
    EntityType& last        = m_entities.back();
    m_indexer->put(last, index);
    m_indexer->put(entity, std::numeric_limits<std::size_t>::max());
    std::swap(theEntity, last);
    m_entities.pop_back();
}
template <class EntityType>
constexpr std::size_t SystemWithDeletion<EntityType>::getSize() const
{
    return m_entities.size();
}
template <class EntityType>
auto SystemWithDeletion<EntityType>::getRange() const
{
    return ranges::make_subrange(m_entities.begin(), m_entities.end());
}
template <class EntityType>
void SystemWithDeletion<EntityType>::doReserve(std::size_t size)
{
    m_entities.reserve(size);
}
template <class EntityType>
void SystemWithDeletion<EntityType>::doAdd()
{
    auto newIndex = m_indexer->allocate();
    m_entities.emplace_back(newIndex);
    m_indexer->put(m_entities.back(), m_entities.size()-1);
}
template <class EntityType>
bool SystemWithDeletion<EntityType>::isAlive(EntityType entity) const
{
    return entity != EntityType{} && m_indexer->lookup(entity) != std::numeric_limits<std::size_t>::max();
}
template <class EntityType>
std::shared_ptr<typename SystemWithDeletion<EntityType>::Indexer> SystemWithDeletion<EntityType>::getIndexer() const
{
    return m_indexer;
}
template <class EntityType>
std::size_t SystemWithDeletion<EntityType>::getCapacity() const
{
    return m_entities.capacity();
}

template <class Derived>
Base<Derived>::Base() :
    m_id(std::numeric_limits<std::size_t>::max())
{
    
}
template <class Derived>
Base<Derived>::Base(std::size_t id) :
    m_id(id)
{
    
}
template <class Derived>
Base<Derived>::~Base()
{
    
}
template <class Derived>
Base<Derived>::Base(const Base& other) :
    m_id(other.m_id)
{
}
template <class Derived>
bool Base<Derived>::operator==(const Base<Derived>& other) const
{
    return m_id == other.m_id;
}
template <class Derived>
bool Base<Derived>::operator!=(const Base<Derived>& other) const
{
    return m_id != other.m_id;
}
template <class Derived>
std::size_t Base<Derived>::id() const
{
    return m_id;
}
template <class Derived>
bool Base<Derived>::operator<(const Base& other) const
{
    return m_id < other.m_id;
}
template <template <typename> class BaseType, class EntityType>
constexpr SystemBase<BaseType, EntityType>::SystemBase() :
    notifier(std::make_shared<Notifier>()),
    m_next(0)
{
}
template <template <typename> class BaseType, class EntityType>
void SystemBase<BaseType, EntityType>::reserve(std::size_t size)
{
	notifier->onReserve(size);
	static_cast<BaseType<EntityType>*>(this)->doReserve(size);
}
template <template <typename> class BaseType, class EntityType>
EntityType SystemBase<BaseType, EntityType>::add()
{
	auto result = m_next;
	notifier->onAdd(result);
	static_cast<BaseType<EntityType>*>(this)->doAdd();
	m_next = EntityType(m_next.id() + 1);
	return result;
}
template <template <typename> class BaseType, class EntityType>
constexpr std::size_t SystemBase<BaseType, EntityType>::capacity() const
{
	return static_cast<const BaseType<EntityType>*>(this)->getCapacity();
}
template <template <typename> class BaseType, class EntityType>
constexpr std::size_t SystemBase<BaseType, EntityType>::size() const
{
	return static_cast<const BaseType<EntityType>*>(this)->getSize();
}
template <template <typename> class BaseType, class EntityType>
constexpr bool SystemBase<BaseType, EntityType>::empty() const
{
	return size() == 0;
}
template <template <typename> class BaseType, class EntityType>
bool SystemBase<BaseType, EntityType>::alive(EntityType entity) const
{
	return static_cast<const BaseType<EntityType>*>(this)->isAlive(entity);
}
template <template <typename> class BaseType, class EntityType>
auto SystemBase<BaseType, EntityType>::indexer() const
{
	return static_cast<const BaseType<EntityType>*>(this)->getIndexer();
}
template <template <typename> class BaseType, class EntityType>
auto SystemBase<BaseType, EntityType>::asRange() const
{
	return static_cast<const BaseType<EntityType>*>(this)->getRange();
}
template <class EntityType>
void System<EntityType>::doAdd()
{
}
template <class EntityType>
void System<EntityType>::doReserve(std::size_t capacity)
{
    m_capacity = capacity;
}
template <class EntityType>
constexpr std::size_t System<EntityType>::getSize() const
{
    return getIndexer()->lookup(this->m_next);
}
template <class EntityType>
bool System<EntityType>::isAlive(EntityType entity) const
{
    return entity.id() < getSize();
}
template <class EntityType>
std::shared_ptr<typename System<EntityType>::Indexer> System<EntityType>::getIndexer() const
{
    static std::shared_ptr<Indexer> staticIndexer{std::make_shared<Indexer>()};
    return staticIndexer;
}
template <class EntityType>
auto System<EntityType>::getRange() const
{
    return ranges::view::iota(static_cast<std::size_t>(0), getSize()) | ranges::view::transform([](auto id){ return EntityType{id}; });
}
template <class EntityType>
std::size_t System<EntityType>::getCapacity() const
{
    return std::max(getSize(), m_capacity);
}
template <class EntityType>
System<EntityType>::System() :
    SystemBase<::Entity::System, EntityType>::SystemBase(),
    m_capacity(0)
{
    
}
template <class EntityType>
std::size_t System<EntityType>::Indexer::lookup(EntityType en) const
{
    return en.id();
}

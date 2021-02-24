template <typename KeyType, typename ValueType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType>::Property()
{
    
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType>::Property(Property&& other):
    Property()
{
    using std::swap;
    swap(*this, other);
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType>::Property(const Property& other):
    m_indexer(other.m_indexer),
    m_notifier(other.m_notifier),
    m_values(other.m_values)
{
    connectSignals();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
    Property<KeyType, ValueType, SystemType>::Property(SystemType<KeyType>& sys):
    m_indexer(sys.indexer()),
    m_notifier(sys.notifier)
{
    m_values.reserve(sys.capacity());
    m_values.resize(sys.size());
    connectSignals();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType>& Property<KeyType, ValueType, SystemType>::operator=(Property&& other)
{
    using std::swap;
    swap(*this, other);
    return *this;
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
Property<KeyType, ValueType, SystemType>& Property<KeyType, ValueType, SystemType>::operator=(const Property& other)
{
    Property copy(other);
    return *this = std::move(copy);
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
constexpr typename std::vector<ValueType>::size_type Property<KeyType, ValueType, SystemType>::size() const
{
    return m_values.size();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
bool Property<KeyType, ValueType, SystemType>::empty() const
{
    return m_values.empty();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
constexpr typename std::vector<ValueType>::size_type Property<KeyType, ValueType, SystemType>::capacity() const
{
    return m_values.capacity();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
typename std::vector<ValueType>::reference Property<KeyType, ValueType, SystemType>::operator[](KeyType key)
{
    return m_values[m_indexer->lookup(key)];
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
typename std::vector<ValueType>::const_reference Property<KeyType, ValueType, SystemType>::operator[](KeyType key) const
{
    return m_values[m_indexer->lookup(key)];
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
auto Property<KeyType, ValueType, SystemType>::asRange()
{
    return ranges::make_subrange(m_values.begin(), m_values.end());
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
auto Property<KeyType, ValueType, SystemType>::asRange() const
{
    return ranges::make_subrange(m_values.cbegin(), m_values.cend());
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
const ValueType* Property<KeyType, ValueType, SystemType>::data() const
{
    return m_values.data();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
void Property<KeyType, ValueType, SystemType>::disconnectOnErase()
{
    m_onEraseConnection.disconnect();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
template <class RangeType>
Property<KeyType, ValueType, SystemType>& Property<KeyType, ValueType, SystemType>::operator=(RangeType range)
{
    ranges::copy(range, m_values.begin());
    return *this;
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
void Property<KeyType, ValueType, SystemType>::onAdd(KeyType)
{
    m_values.push_back(ValueType{});
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
void Property<KeyType, ValueType, SystemType>::onReserve(std::size_t size)
{
    m_values.reserve(size);
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
void Property<KeyType, ValueType, SystemType>::onErase(KeyType en)
{
    std::swap(m_values.back(), m_values[m_indexer->lookup(en)]);
    m_values.pop_back();
}
template <typename KeyType, typename ValueType, template <typename> class SystemType>
void Property<KeyType, ValueType, SystemType>::connectSignals()
{
    if(auto notifier = m_notifier.lock())
    {
        m_onAddConnection     = std::move(notifier->onAdd.connect([this](KeyType en) {
            this->onAdd(en);
        }));
        m_onReserveConnection = std::move(notifier->onReserve.connect([this](std::size_t size) {
            this->onReserve(size);
        }));
        m_onEraseConnection   = std::move(notifier->onErase.connect([this](KeyType en) {
            this->onErase(en);
        }));
    }
}


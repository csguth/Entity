#ifndef COMPOSITION_HPP
#define COMPOSITION_HPP

#include "Property.hpp"
#include "SystemWithDeletion.hpp"
#include <type_traits>

namespace Entity
{

// Selectors for Mapping System
struct Right {};
struct Left  {};
struct Both  {};

// Mapping types

// Non Mapped type is a dummy mapping type, should never be inherit from.
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
struct NonMapped
{
public:
    NonMapped(ParentSystemType<ParentType>&, ChildSystemType<ChildType>&)
    {

    }
};

// A composition should inherit Right Mapped when it is necessary O(1) mapping from child to parent.
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
class RightMapped
{
public:
    RightMapped(ParentSystemType<ParentType>&, ChildSystemType<ChildType>& child):
        m_parent(makeProperty<ParentType>(child))
    {

    }
    ParentType parent(ChildType child) const
    {
        return m_parent[child];
    }
    void parent(ChildType child, ParentType parent)
    {
        m_parent[child] = parent;
    }
    void addChild(ParentType parent, ChildType child)
    {
        m_parent[child] = parent;
    }
    void removeChild(ParentType parent, ChildType child)
    {
        if(this->parent(child) == parent)
        {
            this->parent(child, ParentType{});
        }
    }
protected:
    Property<ChildType, ParentType, ChildSystemType> m_parent;
};

// SFINAE to connect the signal for erasing child entity >
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossibleForLeftMapped(int, boost::signals2::scoped_connection& connection, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>& notifier, ChildSystemType<ChildType>& child, Property<ParentType, ChildType, ParentSystemType>& firstChild, Property<ChildType, ChildType, ChildSystemType>& nextSibling) -> decltype((void)&ChildSystemType<ChildType>::erase, void())
{
    connection = std::move(notifier->onErase.connect([&](ParentType en)
    {
        for(ChildType curr{firstChild[en]}, next; curr != ChildType{}; curr = next)
        {
            next = nextSibling[curr];
            child.erase(curr);
        }
        firstChild.onErase(en);
    }));
}

template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossibleForLeftMapped(char, boost::signals2::scoped_connection&, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>&, ChildSystemType<ChildType>&, Property<ParentType, ChildType, ParentSystemType>&, Property<ChildType, ChildType, ChildSystemType>&) -> decltype(void(), void())
{}
// <

// A composition should inherit Left Mapped when it is necessary O(1) mapping from parent to children.
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
class LeftMapped
{
public:
    LeftMapped(ParentSystemType<ParentType>& parent, ChildSystemType<ChildType>& child):
        m_childrenSize(makeProperty<std::size_t>(parent)),
        m_firstChild(makeProperty<ChildType>(parent)),
        m_nextSibling(makeProperty<ChildType>(child))
    {
        m_firstChild.disconnectOnErase();
        connectOnEraseIfPossibleForLeftMapped(0, m_onEraseConnection, parent.notifier, child, m_firstChild, m_nextSibling);
    }
    ChildType firstChild(ParentType parent) const
    {
        return m_firstChild[parent];
    }
    ChildType nextSibling(ChildType child) const
    {
        return m_nextSibling[child];
    }
    void firstChild(ParentType parent, ChildType child)
    {
        m_firstChild[parent] = child;
    }
    void nextSibling(ChildType child, ChildType next)
    {
        m_nextSibling[child] = next;
    }
    void addChild(ParentType parent, ChildType child)
    {
        ++m_childrenSize[parent];
        m_nextSibling[child] = m_firstChild[parent];
        m_firstChild[parent] = child;
    }
    std::size_t childrenSize(ParentType parent) const
    {
        return m_childrenSize[parent];
    }
    void disconnectOnErase()
    {
        m_onEraseConnection.disconnect();
    }
    void removeChild(ParentType parent, ChildType child)
    {
        --this->m_childrenSize[parent];
        if(child == this->firstChild(parent))
        {
            this->firstChild(parent, this->nextSibling(child));
        }
        else
        {
            auto prev = this->firstChild(parent);
            auto curr = this->nextSibling(prev);
            while(curr != child && curr != ChildType{})
            {
                prev = curr;
                curr = this->nextSibling(curr);
            }
            if(curr == child)
            {
                this->nextSibling(prev, this->nextSibling(curr));
            }
        }
    }
    class ChildrenView
      : public ranges::view_facade<ChildrenView> {
    private:
        friend ranges::range_access;
        const LeftMapped* m_mapped;
        ranges::semiregular_t<ChildType> m_current;
        struct cursor
        {
        private:
            ChildrenView* m_range;
        public:
            cursor() = default;
            explicit cursor(ChildrenView& range)
                : m_range(&range)
            {}
            void next()
            {
                m_range->next();
            }
            ChildType& read() const noexcept
            {
                return m_range->current();
            }
            bool equal(ranges::default_sentinel_t) const
            {
                return m_range->current() == ChildType{};
            }
            bool equal(const cursor& other) const
            {
                return read() == other.read();
            }
        };
        void next()
        {
            m_current = m_mapped->nextSibling(m_current);
        }
        cursor begin_cursor()
        {
            return cursor{*this};
        }
    public:
        ChildrenView() = default;
        ChildrenView(const LeftMapped& mapped, ParentType parent)
            : m_mapped(&mapped),
              m_current(mapped.firstChild(parent))
        { }
        ChildType& current()
        {
            return m_current;
        }
    };

    auto children(ParentType parent) const
    {
        return ChildrenView(*this, parent);
    }
protected:
    boost::signals2::scoped_connection m_onEraseConnection;
    Property<ParentType, std::size_t, ParentSystemType> m_childrenSize;
    Property<ParentType, ChildType, ParentSystemType> m_firstChild;
    Property<ChildType, ChildType, ChildSystemType> m_nextSibling;
};

// SFINAE to connect the signal for erasing child entity >
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossibleForBothMapped(int, boost::signals2::scoped_connection& connection, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>& notifier, ChildSystemType<ChildType>& child, Property<ParentType, ChildType, ParentSystemType>& firstChild, Property<ChildType, ChildType, ChildSystemType>& nextSibling, Property<ChildType, ParentType, ChildSystemType>& parent) -> decltype((void)&ChildSystemType<ChildType>::erase, void())
{
    connectOnEraseIfPossibleForLeftMapped(0, connection, notifier, child, firstChild, nextSibling);
}

template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossibleForBothMapped(char, boost::signals2::scoped_connection& connection, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>& notifier, ChildSystemType<ChildType>& child, Property<ParentType, ChildType, ParentSystemType>& firstChild, Property<ChildType, ChildType, ChildSystemType>& nextSibling, Property<ChildType, ParentType, ChildSystemType>& parent) -> decltype(void(), void())
{
    connection = std::move(notifier->onErase.connect([&](ParentType en)
    {
        for(ChildType curr{firstChild[en]}, next; curr != ChildType{}; curr = next)
        {
            next = nextSibling[curr];
            parent[curr] = ParentType{};
        }
        firstChild.onErase(en);
    }));
}
// <

// BothMapped is a wrap for both Left and Right Mappings.
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
class BothMapped: public LeftMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>, public RightMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>
{
    using LeftParent = LeftMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>;
    using RightParent = RightMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>;
public:
    BothMapped(ParentSystemType<ParentType>& parent, ChildSystemType<ChildType>& child):
        LeftParent(parent, child),
        RightParent(parent, child)
    {
        LeftParent::disconnectOnErase();
        {
            connectOnEraseIfPossibleForBothMapped(0, this->m_onEraseConnection, parent.notifier, child, this->m_firstChild, this->m_nextSibling, this->m_parent);
        }
        this->m_nextSibling.disconnectOnErase();
        this->m_parent.disconnectOnErase();
        m_onEraseChildConnection = std::move(child.notifier->onErase.connect([&](ChildType child)
        {
            auto theParent = this->parent(child);
            if(parent.alive(theParent))
            {
               removeChild(theParent, child);
            }
            this->m_nextSibling.onErase(child);
            this->m_parent.onErase(child);
        }));
    }
    void addChild(ParentType parent, ChildType child)
    {
       LeftParent::addChild(parent, child);
       RightParent::addChild(parent, child);
    }
    void removeChild(ParentType parent, ChildType child)
    {
        RightParent::removeChild(parent, child);
        LeftParent::removeChild(parent, child);
    }

private:
    boost::signals2::scoped_connection m_onEraseChildConnection;
};

// This Conditional helps to select the right Mapping according to the passed Selector.
template <typename Selector, typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
struct Conditional
{
    using type = typename std::conditional<
                    std::is_same<Selector, Left>::value,
                    LeftMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>,
                    typename std::conditional<
                        std::is_same<Selector, Right>::value,
                        RightMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>,
                        typename std::conditional<
                            std::is_same<Selector, Both>::value,
                            BothMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>,
                            NonMapped<ParentType, ParentSystemType, ChildType, ChildSystemType>
                        >::type
                     >::type
                 >::type;
};

// The Composition class
template <typename Selector, typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
class Composition: public Conditional<Selector, ParentType, ParentSystemType, ChildType, ChildSystemType>::type
{
    using Parent = typename Conditional<Selector, ParentType, ParentSystemType, ChildType, ChildSystemType>::type;
public:
    Composition(ParentSystemType<ParentType>& parentSystem, ChildSystemType<ChildType>& childSystem):
        Parent(parentSystem, childSystem)
    {

    }
    void addChild(ParentType parent, ChildType child)
    {
        Parent::addChild(parent, child);
    }
    void removeChild(ParentType parent, ChildType child)
    {
        Parent::removeChild(parent, child);
    }
};

template <typename Selector, typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
Composition<Selector, ParentType, ParentSystemType, ChildType, ChildSystemType>  makeComposition(ParentSystemType<ParentType>& parentSystem, ChildSystemType<ChildType>& childSystem)
{
    return {parentSystem, childSystem};
}

template<typename EntityType>
class WeakAdapter
{
public:
    using Indexer = typename SystemWithDeletion<EntityType>::Indexer;
    using Notifier = typename SystemWithDeletion<EntityType>::Notifier;
    
    WeakAdapter(SystemWithDeletion<EntityType>& system):
        notifier(system.notifier),
        m_system(system)
    {
    }
    auto indexer()
    {
        return m_system.indexer();
    }
    auto capacity() const
    {
        return m_system.capacity();
    }
    auto size() const
    {
        return m_system.size();
    }
    
    std::shared_ptr<Notifier>& notifier;
    SystemWithDeletion<EntityType>& m_system;
};


}

#endif // COMPOSITION_HPP

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
struct RightMapped
{
    RightMapped(ParentSystemType<ParentType>&, ChildSystemType<ChildType>& child)
        : parent{ makeProperty<ParentType>(child) }
    {}
    void addChild(const ParentType& parent, const ChildType& child)
    {
        this->parent[child] = parent;
    }
    void removeChild(const ParentType& parent, const ChildType& child)
    {
        if (this->parent[child] == parent)
        {
            this->parent[child] = ParentType{};
        }
    }
    
    Property<ChildType, ParentType, ChildSystemType> parent;
};

// SFINAE to connect the signal for erasing child entity >
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossibleForLeftMapped(int, boost::signals2::scoped_connection& connection, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>& notifier, ChildSystemType<ChildType>& child, Property<ParentType, ChildType, ParentSystemType>& firstChild, Property<ChildType, ChildType, ChildSystemType>& nextSibling) -> decltype((void)&ChildSystemType<ChildType>::erase, void())
{
    connection = notifier->onErase.connect([&](ParentType en)
    {
        for(ChildType curr{firstChild[en]}, next; curr != ChildType{}; curr = next)
        {
            next = nextSibling[curr];
            child.erase(curr);
        }
        firstChild.onErase(en);
    });
}

template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossibleForLeftMapped(char, boost::signals2::scoped_connection&, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>&, ChildSystemType<ChildType>&, Property<ParentType, ChildType, ParentSystemType>&, Property<ChildType, ChildType, ChildSystemType>&) -> decltype(void(), void())
{}
// <

// A composition should inherit Left Mapped when it is necessary O(1) mapping from parent to children.
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
struct LeftMapped
{
    LeftMapped(ParentSystemType<ParentType>& parent, ChildSystemType<ChildType>& child)
        : childrenCount { makeProperty<std::size_t>(parent) }
        , firstChild { makeProperty<ChildType>(parent) }
        , nextSibling { makeProperty<ChildType>(child) }
    {
        firstChild.disconnectOnErase();
        connectOnEraseIfPossibleForLeftMapped(0, m_onEraseConnection, parent.notifier, child, firstChild, nextSibling);
    }
    
    void addChild(const ParentType& parent, ChildType child)
    {
        ++childrenCount[parent];
        nextSibling[child] = firstChild[parent];
        firstChild[parent] = std::move(child);
    }
    
    void disconnectOnErase()
    {
        m_onEraseConnection.disconnect();
    }
    void removeChild(const ParentType& parent, const ChildType& child)
    {
        --this->childrenCount[parent];
        if(child == firstChild[parent])
        {
            firstChild[parent] = nextSibling[child];
        }
        else
        {
            auto prev = firstChild[parent];
            auto curr = nextSibling[prev];
            while(curr != child && curr != ChildType{})
            {
                prev = curr;
                curr = nextSibling[curr];
            }
            if(curr == child)
            {
                nextSibling[prev] = nextSibling[curr];
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
            m_current = m_mapped->nextSibling[m_current];
        }
        cursor begin_cursor()
        {
            return cursor{*this};
        }
    public:
        ChildrenView() = default;
        ChildrenView(const LeftMapped& mapped, const ParentType& parent)
          : m_mapped{ &mapped }
          , m_current{ mapped.firstChild[parent] }
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
    
    Property<ParentType, ChildType, ParentSystemType> firstChild;
    Property<ChildType, ChildType, ChildSystemType> nextSibling;
    Property<ParentType, std::size_t, ParentSystemType> childrenCount;
    
protected:
    boost::signals2::scoped_connection m_onEraseConnection;
    
    
    
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
    connection = notifier->onErase.connect([&](ParentType en)
    {
        for(ChildType curr{firstChild[en]}, next; curr != ChildType{}; curr = next)
        {
            next = nextSibling[curr];
            parent[curr] = ParentType{};
        }
        firstChild.onErase(en);
    });
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
            connectOnEraseIfPossibleForBothMapped(0, this->m_onEraseConnection, parent.notifier, child, this->firstChild, this->nextSibling, this->parent);
        }
        this->nextSibling.disconnectOnErase();
        this->parent.disconnectOnErase();
        m_onEraseChildConnection = child.notifier->onErase.connect([&](ChildType child)
        {
            auto theParent = this->parent[child];
            if(parent.alive(theParent))
            {
               removeChild(theParent, child);
            }
            this->nextSibling.onErase(child);
            this->parent.onErase(child);
        });
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
    void addChild(const ParentType& parent, const ChildType& child)
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

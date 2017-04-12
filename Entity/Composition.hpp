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
private:
    Property<ChildType, ParentType, ChildSystemType> m_parent;
};

// SFINAE to connect the signal for erasing child entity >
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
auto connectOnEraseIfPossible(int, boost::signals2::scoped_connection& connection, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>& notifier, ChildSystemType<ChildType>& child, Property<ParentType, ChildType, ParentSystemType>& firstChild, Property<ChildType, ChildType, ChildSystemType>& nextSibling) -> decltype((void)&ChildSystemType<ChildType>::erase, void())
{
    connection = std::move(notifier->connectOnErase([&](ParentType en)
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
auto connectOnEraseIfPossible(char, boost::signals2::scoped_connection&, std::shared_ptr<typename ParentSystemType<ParentType>::Notifier>&, ChildSystemType<ChildType>&, Property<ParentType, ChildType, ParentSystemType>&, Property<ChildType, ChildType, ChildSystemType>&) -> decltype(void(), void())
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
        auto notifier = parent.notifier().lock();
        connectOnEraseIfPossible(0, m_onEraseConnection, notifier, child, m_firstChild, m_nextSibling);
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
    class ChildrenView
      : public ranges::view_facade<ChildrenView> {
    private:
        friend ranges::range_access;
        LeftMapped* m_mapped;
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

            bool equal(ranges::default_sentinel) const {
                return m_range->current() == ChildType{};
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
        ChildrenView(LeftMapped& mapped, ChildType *)
            : m_mapped(&mapped), m_current{}
        {}
        ChildrenView(LeftMapped& mapped, ranges::semiregular<ChildType> *)
            : m_mapped(&mapped), m_current{ranges::in_place}
        {}
    public:
        ChildrenView() = default;
        ChildrenView(LeftMapped& mapped, ParentType parent)
            : m_mapped(&mapped),
              m_current(mapped.firstChild(parent))
        { }
        ChildType& current()
        {
            return m_current;
        }
    };

    auto children(ParentType parent)
    {
        return ChildrenView(*this, parent);
    }
private:
    Property<ParentType, std::size_t, ParentSystemType> m_childrenSize;
    Property<ParentType, ChildType, ParentSystemType> m_firstChild;
    Property<ChildType, ChildType, ChildSystemType> m_nextSibling;
    boost::signals2::scoped_connection m_onEraseConnection;
};



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

    }
    void addChild(ParentType parent, ChildType child)
    {
       LeftParent::addChild(parent, child);
       RightParent::addChild(parent, child);
    }
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
};

template <typename Selector, typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
Composition<Selector, ParentType, ParentSystemType, ChildType, ChildSystemType>  makeComposition(ParentSystemType<ParentType>& parentSystem, ChildSystemType<ChildType>& childSystem)
{
    return {parentSystem, childSystem};
}


}

#endif // COMPOSITION_HPP

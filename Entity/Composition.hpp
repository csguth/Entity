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

// A composition should inherit Left Mapped when it is necessary O(1) mapping from parent to children.
template <typename ParentType, template <typename> class ParentSystemType, typename ChildType, template <typename> class ChildSystemType>
class LeftMapped
{
public:
    LeftMapped(ParentSystemType<ParentType>& parent, ChildSystemType<ChildType>& child):
        m_firstChild(makeProperty<ChildType>(parent)),
        m_nextSibling(makeProperty<ChildType>(child))
    {

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
        m_nextSibling[child] = m_firstChild[parent];
        m_firstChild[parent] = child;
    }
    auto children(ParentType parent)
    {
        std::vector<ChildType> children;
        for(ChildType curr = m_firstChild[parent]; !(curr == ChildType{}); curr = nextSibling(curr))
        {
            children.push_back(curr);
        }
        return children;
    }

private:
    Property<ParentType, ChildType, ParentSystemType> m_firstChild;
    Property<ChildType, ChildType, ChildSystemType> m_nextSibling;
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

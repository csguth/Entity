#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <Entity/Core/Property.hpp>
#include <Entity/Core/Composition.hpp>
#include <deque>

namespace Entity
{

namespace Graph
{

ENTITY_ENTITY_DECLARATION(Vertex);
ENTITY_ENTITY_DECLARATION(Arc);


template <template <typename> class SystemType>
class DigraphBase
{
public:
    using VertexType = Vertex;
    using ArcType = Arc;
    DigraphBase():
        m_vertices(),
        m_arcs(),
        m_inArcs(makeComposition<Both>(m_vertices, m_arcs)),
        m_outArcs(makeComposition<Both>(m_vertices, m_arcs))
    {
    }
    virtual ~DigraphBase()
    {}
    std::size_t order() const
    {
        return m_vertices.size();
    }
    std::size_t size() const
    {
        return m_arcs.size();
    }
    Vertex addVertex()
    {
        return m_vertices.add();
    }
    Arc addArc(const Vertex& source, const Vertex& target)
    {
        const Arc arc = m_arcs.add();
        m_outArcs.addChild(source, arc);
        m_inArcs.addChild(target, arc);
        return arc;
    }
    uint32_t inDegree(const Vertex& v) const
    {
        return m_inArcs.childrenCount[v];
    }
    uint32_t outDegree(const Vertex& u) const
    {
        return m_outArcs.childrenCount[u];
    }
    
    Vertex source(const Arc& arc) const
    {
        return m_outArcs.parent[arc];
    }
    Vertex target(const Arc& arc) const
    {
        return m_inArcs.parent[arc];
    }
    Vertex opposite(Arc arc, Vertex vertex) const
    {
        const auto u = source(arc);
        return (u == vertex ? target(arc) : u);
    }
    Arc arc(Vertex u, Vertex v) const
    {
        if(outDegree(u) < inDegree(v))
        {
            auto theTargetIsV = [v, this](Arc arc)
            {
                return target(arc) == v;
            };
            auto uOutArcs = outArcs(u);
            auto result = ranges::find_if(uOutArcs, theTargetIsV);
            return (result != uOutArcs.end() ? *result : Arc{});
        }
        else
        {
            auto theSourceIsU = [u, this](Arc arc)
            {
                return source(arc) == u;
            };
            auto vInArcs = inArcs(v);
            auto result = ranges::find_if(vInArcs, theSourceIsU);
            return (result != vInArcs.end() ? *result : Arc{});
        }
    }
    template <class ValueType>
    auto makeVertexProperty()
    {
        return makeProperty<ValueType>(m_vertices);
    }
    template <class ValueType>
    auto makeArcProperty()
    {
        return makeProperty<ValueType>(m_arcs);
    }
    auto vertices() const
    {
        return m_vertices.asRange();
    }
    auto arcs() const
    {
        return m_arcs.asRange();
    }
protected:
    SystemType<Vertex> m_vertices;
    SystemType<Arc>    m_arcs;
    decltype(makeComposition<Both>(m_vertices, m_arcs)) m_inArcs;
    decltype(makeComposition<Both>(m_vertices, m_arcs)) m_outArcs;

public:
    auto outArcs(Vertex u) const -> decltype(this->m_inArcs.children(Vertex{}))
    {
        return m_outArcs.children(u);
    }
    auto inArcs(Vertex v) const -> decltype(this->m_inArcs.children(Vertex{}))
    {
        return m_inArcs.children(v);
    }
};

class SmartDigraph: public DigraphBase<System>
{
public:
    using Parent = DigraphBase<System>;
    using Parent::VertexType;
    using Parent::ArcType;
};

class Digraph: public DigraphBase<SystemWithDeletion>
{
public:
    using Parent = DigraphBase<SystemWithDeletion>;
    using Parent::VertexType;
    using Parent::ArcType;
    using Parent::DigraphBase;
    void erase(Vertex v)
    {
        m_vertices.erase(v);
    }
    void erase(Arc uv)
    {
        m_arcs.erase(uv);
    }
};

class SmartGraph: private SmartDigraph
{
public:
    using Parent = SmartDigraph;
    using Parent::Parent;
    using Parent::VertexType;
    using Parent::ArcType;
    using Parent::DigraphBase;
    using Parent::addVertex;
    using Edge = std::pair<Arc, Arc>;
    Edge addEdge(Vertex u, Vertex v)
    {
        return {Parent::addArc(u, v), Parent::addArc(v, u)};
    }
    uint32_t degree(Vertex u) const
    {
        return Parent::inDegree(u);
    }
};

class Graph: private Digraph
{
public:
    using Parent = Digraph;
    using Parent::Parent;
    using Parent::VertexType;
    using Parent::ArcType;
    using Parent::DigraphBase;
    using Parent::addVertex;
    using Edge = std::pair<Arc, Arc>;
    Edge addEdge(Vertex u, Vertex v)
    {
        return {Parent::addArc(u, v), Parent::addArc(v, u)};
    }
    uint32_t degree(Vertex u) const
    {
        return Parent::inDegree(u);
    }
    void erase(Vertex u)
    {
        Parent::erase(u);
    }
    void erase(Edge e)
    {
        Parent::erase(e.first);
        Parent::erase(e.second);
    }
};

class BreadthFirstView
  : public ranges::view_facade<BreadthFirstView>
{
private:
    friend ranges::range_access;
    SmartDigraph* m_SmartDigraph;
    ranges::semiregular_t<Vertex> m_source;
    ranges::semiregular_t<Vertex> m_current;
    struct cursor
    {
    private:
        BreadthFirstView* m_range;
    public:
        cursor() = default;
        explicit cursor(BreadthFirstView& range)
            : m_range(&range)
        {}
        void next()
        {
            m_range->next();
        }
        Vertex& read() const noexcept
        {
            return m_range->current();
        }
        bool equal(ranges::default_sentinel_t) const
        {
            return m_range->current() == Vertex{};
        }
        bool equal(const cursor& other) const
        {
            return read() == other.read();
        }
    };
    void next()
    {
        if(m_Q.empty())
        {
            m_current = Vertex{};
        }
        else
        {
            auto&& front = m_Q.front();
            m_current = front;
            m_colors[front] = Color::Black;
            ranges::for_each(m_SmartDigraph->outArcs(front), [&](Arc a)
            {
                const auto target = m_SmartDigraph->target(a);
                assert(m_colors[target] != Color::Black);
                if(m_colors[target] == Color::White)
                {
                    m_colors[target] = Color::Grey;
                    m_Q.push_back(target);
                }
            });
            m_Q.pop_front();
        }
    }
    cursor begin_cursor()
    {
        ranges::fill(m_colors.asRange(), Color::White);
        m_Q.push_back(m_source);
        cursor c{*this};
        c.next();
        return c;
    }
public:
    BreadthFirstView() = default;
    BreadthFirstView(SmartDigraph& SmartDigraph, Vertex source)
        : m_SmartDigraph(&SmartDigraph),
          m_source(source),
          m_colors(SmartDigraph.makeVertexProperty<Color>())
    {
    }
    Vertex& current()
    {
        return m_current;
    }
private:
    enum class Color
    {
        Black,
        Grey,
        White
    };
    decltype(m_SmartDigraph->makeVertexProperty<Color>()) m_colors;
    std::deque<Vertex> m_Q;
};

auto bfs(SmartDigraph& d, Vertex source)
{
    return BreadthFirstView(d, source);
}


}
}

namespace ranges
{
namespace view
{
    /*
     * struct Node
     * {
     *      SmartDigraph& graph;
     *      Vertex u;
     *      bool canReach(Vertex v) const
     *      {
     *          return count(view::bfs(graph, v), v) > 0;
     *      }
     *      auto neighborhood() const
     *      {
     *          return view::concat(graph.outArcs(u) | view::transform(target),
     *                              graph.inArcs(u) | view::transform(source));
     *      }
     * };
     */
    template <typename GraphType, typename SourceVertex>
    auto bfs(GraphType& graph, SourceVertex source)
    {
        return Entity::Graph::BreadthFirstView(graph, source);
    }
}
}

#endif // GRAPH_HPP

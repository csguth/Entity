#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <Entity/Core/Property.hpp>
#include <Entity/Core/Composition.hpp>
#include <deque>

namespace Entity
{

namespace Graph
{

struct Vertex: Base<Vertex>
{
    using Base<Vertex>::Base;
    static std::string name()
    {
        return "Vertex";
    }
};

struct Arc: Base<Arc>
{
    using Base<Arc>::Base;
    static std::string name()
    {
        return "Arc";
    }
};

class Digraph
{
public:
    using VertexType = Vertex;
    using ArcType = Arc;
    Digraph():
        m_vertices(),
        m_arcs(),
        m_inArcs(makeComposition<Both>(m_vertices, m_arcs)),
        m_outArcs(makeComposition<Both>(m_vertices, m_arcs))
    {
    }
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
    Arc addArc(Vertex source, Vertex target)
    {
        const Arc arc = m_arcs.add();
        m_outArcs.addChild(source, arc);
        m_inArcs.addChild(target, arc);
        return arc;
    }
    uint32_t inDegree(Vertex v) const
    {
        return m_inArcs.childrenSize(v);
    }
    uint32_t outDegree(Vertex u) const
    {
        return m_outArcs.childrenSize(u);
    }
    Vertex source(Arc arc) const
    {
        return m_outArcs.parent(arc);
    }
    Vertex target(Arc arc) const
    {
        return m_inArcs.parent(arc);
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
private:
    System<Vertex> m_vertices;
    System<Arc>    m_arcs;
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

class Graph: private Digraph
{
public:
    using Digraph::Digraph;
    using Edge = std::pair<Arc, Arc>;
    using Digraph::addVertex;
    Edge addEdge(Vertex u, Vertex v)
    {
        return {Digraph::addArc(u, v), Digraph::addArc(v, u)};
    }
    uint32_t degree(Vertex u) const
    {
        return Digraph::inDegree(u);
    }
};

class BreadthFirstView
  : public ranges::view_facade<BreadthFirstView> {
private:
    friend ranges::range_access;
    Digraph* m_digraph;
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
        bool equal(ranges::default_sentinel) const
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
            ranges::for_each(m_digraph->outArcs(front), [&](Arc a)
            {
                const auto target = m_digraph->target(a);
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
    BreadthFirstView(Digraph& digraph, Vertex source)
        : m_digraph(&digraph),
          m_source(source),
          m_colors(digraph.makeVertexProperty<Color>())
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
    decltype(m_digraph->makeVertexProperty<Color>()) m_colors;
    std::deque<Vertex> m_Q;
};

auto bfs(Digraph& d, Vertex source)
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
     *      Digraph& graph;
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

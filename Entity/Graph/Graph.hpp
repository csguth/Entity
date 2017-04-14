#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <Entity/Core/Property.hpp>
#include <Entity/Core/Composition.hpp>

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

#include <deque>
template<class Callback>
void breadthFirstSearch(Digraph& graph, Vertex source, Callback&& cb)
{
    using namespace ranges::v3;
    enum class Color
    {
        Black,
        Grey,
        White
    };
    auto colors = graph.makeVertexProperty<Color>();
    fill(colors.asRange(), Color::White);
    std::deque<Vertex> Q;
    Q.push_back(std::move(source));
    while(!Q.empty())
    {
        auto&& front = Q.front();
        cb(front);
        colors[front] = Color::Black;
        for_each(graph.outArcs(front), [&](Arc a)
        {
            const auto target = graph.target(a);
            assert(colors[target] != Color::Black);
            if(colors[target] == Color::White)
            {
                colors[target] = Color::Grey;
                Q.push_back(target);
            }
        });
        Q.pop_front();
    }
}

}
}

#endif // GRAPH_HPP

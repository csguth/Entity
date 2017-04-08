#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <deque>
#include <Entity/Property.hpp>
#include <boost/serialization/strong_typedef.hpp>

namespace Example
{

BOOST_STRONG_TYPEDEF(Entity::Base, Vertex)
BOOST_STRONG_TYPEDEF(Entity::Base, Arc)

class Digraph
{
public:
    Digraph():
        m_firstInArc(m_vertices),
        m_firstOutArc(m_vertices),
        m_inDegree(m_vertices),
        m_outDegree(m_vertices),
        m_nextInArc(m_arcs),
        m_nextOutArc(m_arcs),
        m_source(m_arcs),
        m_target(m_arcs)
    {

    }
    Vertex addVertex()
    {
        return m_vertices.add();
    }
    Arc addArc(Vertex source, Vertex target)
    {
        const Arc arc         = m_arcs.add();
        m_nextOutArc[arc]     = m_firstOutArc[source];
        m_firstOutArc[source] = arc;
        m_nextInArc[arc]      = m_firstInArc[target];
        m_firstInArc[target]  = arc;
        m_source[arc]         = source;
        m_target[arc]         = target;
        ++m_inDegree[target];
        ++m_outDegree[source];
        return arc;
    }
    uint32_t inDegree(Vertex v) const
    {
        return m_inDegree[std::move(v)];
    }
    uint32_t outDegree(Vertex u) const
    {
        return m_outDegree[std::move(u)];
    }
    Vertex source(Arc arc) const
    {
        return m_source[std::move(arc)];
    }
    Vertex target(Arc arc) const
    {
        return m_target[std::move(arc)];
    }
    Arc firstInArc(Vertex v) const
    {
        return m_firstInArc[std::move(v)];
    }
    Arc nextInArc(Arc arc) const
    {
        return m_nextInArc[std::move(arc)];
    }
    Arc firstOutArc(Vertex v) const
    {
        return m_firstOutArc[std::move(v)];
    }
    Arc nextOutArc(Arc arc) const
    {
        return m_nextOutArc[std::move(arc)];
    }
    template <class ValueType>
    auto makeVertexProperty()
    {
        return Entity::makeProperty<ValueType>(m_vertices);
    }
    template <class ValueType>
    auto makeArcProperty()
    {
        return Entity::makeProperty<ValueType>(m_arcs);
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
    Entity::System<Vertex>                             m_vertices;
    Entity::Property<Vertex, Arc, Entity::System>      m_firstInArc;
    Entity::Property<Vertex, Arc, Entity::System>      m_firstOutArc;
    Entity::Property<Vertex, uint32_t, Entity::System> m_inDegree;
    Entity::Property<Vertex, uint32_t, Entity::System> m_outDegree;
    Entity::System<Arc>                                m_arcs;
    Entity::Property<Arc, Arc, Entity::System>         m_nextInArc;
    Entity::Property<Arc, Arc, Entity::System>         m_nextOutArc;
    Entity::Property<Arc, Vertex, Entity::System>      m_source;
    Entity::Property<Arc, Vertex, Entity::System>      m_target;
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


template<class Callback>
void breadthFirstSearch(Digraph& graph, Vertex source, Callback&& cb)
{
    enum class Color
    {
        Black,
        Grey,
        White
    };
    auto colors = graph.makeVertexProperty<Color>();
    ranges::for_each(colors.asRange(), [](Color& color){
       color = Color::White;
    });
    std::deque<Vertex> Q;
    Q.push_back(std::move(source));
    while(!Q.empty())
    {
        auto&& front = Q.front();
        cb(front);
        colors[front] = Color::Black;
        for(Arc arc = graph.firstOutArc(front); arc != Arc{}; arc = graph.nextOutArc(arc))
        {
            switch(colors[graph.target(arc)])
            {
            case Color::White:
                colors[graph.target(arc)] = Color::Grey;
                Q.push_back(graph.target(arc));
                break;
            case Color::Grey:
                break;
            case Color::Black:
                assert(false);
                break;
            }
        }
        Q.pop_front();
    }
}

}

#endif // GRAPH_HPP

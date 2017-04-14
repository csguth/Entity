#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <range/v3/algorithm.hpp>
#include <limits>
#include <queue>
#include <algorithm>

namespace Entity
{
namespace Graph
{

template <class GraphType_>
class Dijkstra
{
public:
    using GraphType = GraphType_;
    using ArcPropertyType = decltype(GraphType(). template makeArcProperty<int>());
    template <class ValueType>
    using VertexPropertyType = decltype(GraphType(). template makeVertexProperty<ValueType>());
    struct Priority
    {
        const VertexPropertyType<int>& dist;
        bool operator()(typename GraphType::VertexType a, typename GraphType::VertexType b) const
        {
            return dist[a] < dist[b];
        }
    };
    static constexpr int inf()
    {
        return std::numeric_limits<int>::max();
    }
    Dijkstra(GraphType& graph, const ArcPropertyType& weigths):
        m_graph(graph),
        m_weigths(weigths),
        m_dist(m_graph. template makeVertexProperty<int>()),
        m_prev(m_graph. template makeVertexProperty<typename GraphType::ArcType>())
    {

    }
    void run(typename GraphType::VertexType source)
    {
        namespace rng = ranges::v3;
        rng::fill(m_dist.asRange(), inf());
        rng::fill(m_prev.asRange(), typename GraphType::ArcType{});
        m_dist[source] = 0;

        std::priority_queue<typename GraphType::VertexType,
                            std::deque<typename GraphType::VertexType>,
                            Priority> Q({m_dist});
        rng::for_each(m_graph.vertices(), [&](auto vertex)
        {
            Q.push(vertex);
        });
        while(!Q.empty())
        {
            typename GraphType::VertexType u = Q.top();
            Q.pop();
            if(m_dist[u] == inf()) continue;
            rng::for_each(m_graph.outArcs(u), [&](typename GraphType::ArcType uv)
            {
                auto v = m_graph.target(uv);
                auto alt = m_dist[u] + m_weigths[uv];
                if(alt < m_dist[v])
                {
                    m_dist[v] = alt;
                    m_prev[v] = uv;
                    Q.push(v);
                }
            });
        }
    }
    auto&& dist() const
    {
        return m_dist;
    }
    auto&& prev() const
    {
        return m_prev;
    }
    auto&& graph() const
    {
        return m_graph;
    }
private:
    GraphType& m_graph;
    const ArcPropertyType& m_weigths;
    VertexPropertyType<int> m_dist;
    VertexPropertyType<typename GraphType::ArcType> m_prev;
};


template <typename GraphType>
class ShortestPathView
    : public ranges::view_facade<ShortestPathView<GraphType>>
{
private:
    friend ranges::range_access;
    const Dijkstra<GraphType>* m_dijkstra;
    typename GraphType::VertexType m_target;
    ranges::semiregular_t<typename GraphType::ArcType> m_current;
    struct cursor
    {
    private:
        ShortestPathView* m_view;
    public:
        cursor() = default;
        explicit cursor(ShortestPathView& view)
            : m_view(&view)
        {}
        void next()
        {
            m_view->next();
        }
        typename GraphType::ArcType& read() const noexcept
        {
            return m_view->current();
        }
        bool equal(ranges::default_sentinel) const
        {
            return m_view->current() == typename GraphType::ArcType{};
        }
    };
    void next()
    {
        if(m_current == typename GraphType::ArcType{}) return;
        m_current = m_dijkstra->prev()[m_dijkstra->graph().source(m_current)];
    }
    cursor begin_cursor()
    {
        m_current = m_dijkstra->prev()[m_target];
        return cursor{*this};
    }

public:
    ShortestPathView() = default;
    ShortestPathView(const Dijkstra<GraphType>& dijkstra, typename GraphType::VertexType target):
        m_dijkstra(&dijkstra),
        m_target(target)
    {}
    typename GraphType::ArcType& current()
    {
        return m_current;
    }
    auto weight()
    {
       return m_dijkstra->dist()[m_target];
    }

private:

};

}
}




#endif // DIJKSTRA_HPP

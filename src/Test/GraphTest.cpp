#define CATCH_CONFIG_MAIN

#include <sstream>
#include <catch.hpp>

#include <Entity/Graph/Graph.hpp>

using namespace Entity::Graph;

TEST_CASE("Empty SmartDigraph")
{
    SmartDigraph d;
    CHECK(d.order() == 0);
    CHECK(d.size() == 0);
}

TEST_CASE("Add Vertex")
{
    SmartDigraph d;
    d.addVertex();
    CHECK(d.order() == 1);
}

TEST_CASE("Add Arc")
{
    SmartDigraph d;
    d.addArc(d.addVertex(), d.addVertex());
    CHECK(d.size() == 1);
}

TEST_CASE("Source, Target, Opposite")
{
    SmartDigraph d;
    auto source = d.addVertex();
    auto target = d.addVertex();
    auto arc = d.addArc(source, target);
    CHECK(d.source(arc) == source);
    CHECK(d.target(arc) == target);
    CHECK(d.opposite(arc, source) == target);
    CHECK(d.opposite(arc, target) == source);
}

TEST_CASE("Get arc")
{
    SmartDigraph d;
    auto source = d.addVertex();
    auto target = d.addVertex();
    auto arc = d.addArc(source, target);
    CHECK(arc == d.arc(source, target));
    CHECK(Arc{} == d.arc(target, source));
}

TEST_CASE("Degree")
{
    /*
        v0 -e0-> v1 -e1-> v2
           -e2-> v3
    */
    SmartDigraph d;
    auto v0 = d.addVertex();
    auto v1 = d.addVertex();
    auto v2 = d.addVertex();
    auto v3 = d.addVertex();

    auto e0 = d.addArc(v0, v1);
    auto e1 = d.addArc(v1, v2);
    auto e2 = d.addArc(v0, v3);

    CHECK(d.inDegree(v0) == 0);
    CHECK(d.inDegree(v1) == 1);
    CHECK(d.inDegree(v2) == 1);
    CHECK(d.inDegree(v3) == 1);

    CHECK(d.outDegree(v0) == 2);
    CHECK(d.outDegree(v1) == 1);
    CHECK(d.outDegree(v2) == 0);
    CHECK(d.outDegree(v3) == 0);
}

namespace ranges {
template <class T, class U>
std::ostream& operator << (std::ostream& os, common_pair<T, U> const& p) {
  return os << '(' << p.first << ", " << p.second << ')';
}
}

TEST_CASE("BFS")
{
    using namespace ranges::v3;
    /*
        v0 -e0-> v1 -e1-> v2
           -e2-> v3 -e3-> v4
                    -e4-> v5 -e5-> v6
    */
    SmartDigraph d;
    auto v0 = d.addVertex();
    auto v1 = d.addVertex();
    auto v2 = d.addVertex();
    auto v3 = d.addVertex();
    auto v4 = d.addVertex();
    auto v5 = d.addVertex();
    auto v6 = d.addVertex();
    auto e0 = d.addArc(v0, v1);
    auto e1 = d.addArc(v1, v2);
    auto e2 = d.addArc(v0, v3);
    auto e3 = d.addArc(v3, v4);
    auto e4 = d.addArc(v3, v5);
    auto e5 = d.addArc(v5, v6);

    CHECK(count(bfs(d, v0), v6) > 0);
    CHECK(count(bfs(d, v5), v0) == 0);
}

#include <Entity/Graph/Dijkstra.hpp>
#include <range/v3/view/generate.hpp>
#include <range/v3/view/zip.hpp>

template <typename GraphType>
auto dijkstraBoilerplate()
{
    GraphType d;
    auto weights = d. template makeArcProperty<int>();
    auto names   = d. template makeVertexProperty<std::string>();
    std::map<std::string, Vertex> vertices;

    auto addOrGetVertex = [&](std::string name)
    {
        auto result = vertices.find(name);
        if(result != vertices.end())
        {
            return result->second;
        }
        auto vertex = d.addVertex();
        vertices[name] = vertex;
        names[vertex] = name;
        return vertex;
    };

    auto addArc = [&](std::string u, std::string v, int weight)
    {
        auto uVertex = addOrGetVertex(u);
        auto vVertex = addOrGetVertex(v);
        auto arc = d.addArc(uVertex, vVertex);
        weights[arc] = weight;
        return arc;
    };

    addArc("a", "b", 7);
    addArc("a", "c", 9);
    addArc("a", "f", 14);
    addArc("b", "c", 10);
    addArc("b", "d", 15);
    addArc("c", "d", 11);
    addArc("c", "f", 2);
    addArc("d", "e", 6);
    addArc("e", "f", 9);

    Dijkstra<GraphType> dijkstra(d, weights);
    dijkstra.run(addOrGetVertex("a"));

    ShortestPathView<GraphType> sp(dijkstra, addOrGetVertex("e"));
    CHECK(sp.weight() == 26);

    namespace rng = ranges::v3;
    const std::vector<std::string> goldenSp{{"e", "d", "c", "a"}};
    const std::vector<std::string> shortestPath = [&]()
    {
        auto source = rng::view::transform([&](typename GraphType::ArcType arc) -> typename GraphType::VertexType
        {
            return d.source(arc);
        });
        const std::vector<std::string> result = [&]()
        {
            std::vector<std::string> result{{"e"}};
            rng::copy(sp | source | rng::view::get(names), ranges::back_inserter(result));
            return result;
        }();
        return result;
    }();
    CHECK(goldenSp == shortestPath);
}

TEST_CASE("Dijkstra")
{
    dijkstraBoilerplate<SmartDigraph>();
    dijkstraBoilerplate<Digraph>();
//    dijkstraBoilerplate<Graph>(); TODO: make this work
//    dijkstraBoilerplate<SmartGraph>();
}


TEST_CASE("Digraph with Deletion")
{
    Digraph d;
    CHECK(d.order() == 0);
    CHECK(d.size() == 0);
    auto vertex = d.addVertex();
    CHECK(d.order() == 1);
    d.erase(vertex);
    CHECK(d.order() == 0);
}

TEST_CASE("Digraph/ Should delete arcs when vertex is deleted")
{
    Digraph d;
    auto u = d.addVertex();
    auto v = d.addVertex();
    auto e = d.addArc(u, v);
    d.erase(v);
    CHECK(d.outDegree(u) == 0);
    CHECK(d.size() == 0);
}

TEST_CASE("Digraph/ Should keep vertices when arc is deleted")
{
    Digraph d;
    auto u = d.addVertex();
    auto v = d.addVertex();
    auto e = d.addArc(u, v);
    d.erase(e);
    CHECK(d.outDegree(u) == 0);
    CHECK(d.inDegree(v) == 0);
    CHECK(d.size() == 0);
    CHECK(d.order() == 2);
}

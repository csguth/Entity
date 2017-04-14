#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <sstream>
#include "Graph.hpp"

using namespace Entity::Graph;

TEST_CASE("Empty Digraph")
{
    Digraph d;
    CHECK(d.order() == 0);
    CHECK(d.size() == 0);
}

TEST_CASE("Add Vertex")
{
    Digraph d;
    d.addVertex();
    CHECK(d.order() == 1);
}

TEST_CASE("Add Arc")
{
    Digraph d;
    d.addArc(d.addVertex(), d.addVertex());
    CHECK(d.size() == 1);
}

TEST_CASE("Source, Target, Opposite")
{
    Digraph d;
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
    Digraph d;
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
    Digraph d;
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
    Digraph d;
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

#include "Dijkstra.hpp"
#include <range/v3/view/generate.hpp>
#include <range/v3/view/zip.hpp>
TEST_CASE("Dijkstra")
{
    Digraph d;
    auto v0 = d.addVertex();
    auto v1 = d.addVertex();
    auto v2 = d.addVertex();
    auto v3 = d.addVertex();
    auto v4 = d.addVertex();
    auto v5 = d.addVertex();
    auto v6 = d.addVertex();
    auto vInf = d.addVertex();
    auto e0 = d.addArc(v0, v1);
    auto e1 = d.addArc(v1, v2);
    auto e2 = d.addArc(v0, v3);
    auto e3 = d.addArc(v3, v4);
    auto e4 = d.addArc(v3, v5);
    auto e5 = d.addArc(v5, v6);
    auto cost = d.makeArcProperty<int>();
    int i = 0;
    auto generator = ranges::v3::view::generate([&](){return ++i;});
    cost = (generator | ranges::v3::view::take(6));
    Dijkstra<Digraph> dijkstra(d, cost);
    dijkstra.run(v0);
    std::cout << ranges::v3::view::zip(d.vertices(),
                                       dijkstra.dist().asRange()) << std::endl;

}

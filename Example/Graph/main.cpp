#include <iostream>
#include "Graph.hpp"

using namespace Example;

int main(int, char *[])
{
    {
        Digraph digraph;
        auto v0 = digraph.addVertex();
        auto v1 = digraph.addVertex();
        auto v2 = digraph.addVertex();
        auto v3 = digraph.addVertex();
        auto v4 = digraph.addVertex();
        auto v5 = digraph.addVertex();
        digraph.addArc(v0, v1);
        digraph.addArc(v1, v2);
        digraph.addArc(v1, v3);
        digraph.addArc(v1, v5);
        digraph.addArc(v3, v4);
        breadthFirstSearch(digraph, v0, [&digraph](Vertex v0)
        {
            std::cout << "Vertex " << static_cast<Entity::Base>(v0).id() << " has degree in " << digraph.inDegree(v0) << " out " << digraph.outDegree(v0) << std::endl;
        });
    }

    Graph graph;
    auto v0 = graph.addVertex();
    auto v1 = graph.addVertex();
    auto v2 = graph.addVertex();
    graph.addEdge(v0, v1);
    graph.addEdge(v1, v2);
    std::cout << "Vertex " << static_cast<Entity::Base>(v0).id() << " has degree " << graph.degree(v0) << std::endl;
    std::cout << "Vertex " << static_cast<Entity::Base>(v1).id() << " has degree " << graph.degree(v1) << std::endl;
    std::cout << "Vertex " << static_cast<Entity::Base>(v2).id() << " has degree " << graph.degree(v2) << std::endl;

    return 0;
}

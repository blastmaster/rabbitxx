#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/trace/simple_graph_builder.hpp>
#include <rabbitxx/graph.hpp>

TEST_CASE("create empty SimpleGraph", "[SimpleGraph]")
{
    rabbitxx::SimpleGraph g;
    auto n_v = g.num_vertices();
    auto n_e = g.num_edges();
    REQUIRE(n_v == 0);
    REQUIRE(n_e == 0);
}

TEST_CASE("vertex iterators of empty graph should be equal", "[SimpleGraph]")
{
    using rabbitxx::SimpleGraph;
    SimpleGraph g;
    SimpleGraph::vertex_range v_r = g.vertices();
    REQUIRE(v_r.first == v_r.second);
}

TEST_CASE("edge iterators of empty graph should be equal", "[SimpleGraph]")
{
    using rabbitxx::SimpleGraph;
    SimpleGraph g;
    SimpleGraph::edge_range e_r = g.edges();
    REQUIRE(e_r.first == e_r.second);
}

TEST_CASE("empty graph has empty list of vertex descriptors", "[SimpleGraph]")
{
    using rabbitxx::SimpleGraph;
    SimpleGraph g;
    auto vec_vd = g.vertex_descriptors();
    REQUIRE(vec_vd.size() == 0);
}

TEST_CASE("empyt graph has empty list of edge descriptors", "[SimpleGraph]")
{
    using rabbitxx::SimpleGraph;
    SimpleGraph g;
    auto vec_ed = g.edge_descriptors();
    REQUIRE(vec_ed.size() == 0);
}

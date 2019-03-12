#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/graph.hpp>

TEST_CASE("create empty SimpleGraph", "[SimpleGraph]")
{
    rabbitxx::IoGraph g;
    auto n_v = g.num_vertices();
    auto n_e = g.num_edges();
    REQUIRE(n_v == 0);
    REQUIRE(n_e == 0);
}

TEST_CASE("vertex iterators of empty graph should be equal", "[SimpleGraph]")
{
    using rabbitxx::IoGraph;
    IoGraph g;
    IoGraph::vertex_range v_r = g.vertices();
    REQUIRE(v_r.first == v_r.second);
}

TEST_CASE("edge iterators of empty graph should be equal", "[SimpleGraph]")
{
    using rabbitxx::IoGraph;
    IoGraph g;
    IoGraph::edge_range e_r = g.edges();
    REQUIRE(e_r.first == e_r.second);
}

TEST_CASE("empty graph has empty list of vertex descriptors", "[SimpleGraph]")
{
    using rabbitxx::IoGraph;
    IoGraph g;
    auto vec_vd = g.vertex_descriptors();
    REQUIRE(vec_vd.empty());
}

TEST_CASE("empty graph has empty list of edge descriptors", "[SimpleGraph]")
{
    using rabbitxx::IoGraph;
    IoGraph g;
    auto vec_ed = g.edge_descriptors();
    REQUIRE(vec_ed.empty());
}

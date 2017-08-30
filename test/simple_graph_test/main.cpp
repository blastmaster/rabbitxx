
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/trace/simple_graph_builder.hpp>
#include <rabbitxx/graph.hpp>

TEST_CASE("create empty SimpleGraph", "[SimpleGraph]")
{
    rabbitxx::SimpleGraph g;
    auto n_v = g.num_vertices();
    REQUIRE(n_v == 0);
}

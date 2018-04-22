#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

/**
 * FIXME:
 * The `make_local_pgmap` function is deprecated and removed.
 * Use the root events of the synchronization stored somewhere.
 */

TEST_CASE("[trace-simple]", "Find local Process Groups of Synchronization Events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/24.10/trace-simple/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    //auto lpgs = rabbitxx::make_local_pgmap(*graph);


    // here we have just two processes therefore every synchronization is
    // global! so for *local* process groups we get the empty set!
    rabbitxx::pg_map_t<rabbitxx::VertexDescriptor> exp_local_pgmap { };

    REQUIRE(exp_local_pgmap == lpgs);
}

TEST_CASE("[trace-own]", "Find local Process Groups of Synchronization Events")
{
    static const std::string trc_file {"/home/soeste/traces/trace-own_trace-20171116_1704_50058813361150354/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    //auto lpgs = rabbitxx::make_local_pgmap(*graph);


    rabbitxx::pg_map_t<rabbitxx::VertexDescriptor> exp_local_pgmap {
        { 12, { 0, 2 } },
        { 17, { 0, 3 } },
        { 18, { 0, 1 } }
    };

    REQUIRE(exp_local_pgmap == lpgs);
}

TEST_CASE("[ec]", "Find local Process Groups of Synchronization Events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-edgecase/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    //auto lpgs = rabbitxx::make_local_pgmap(*graph);


    rabbitxx::pg_map_t<rabbitxx::VertexDescriptor> exp_local_pgmap {
        { 19, { 0, 1 } },
        { 21, { 2, 3 } },
        { 22, { 0, 1 } },
        { 26, { 2, 3 } }
    };

    std::cout << "local pg_map_t: " << std::endl;
    for (const auto& pg : lpgs)
    {
        std::cout << "[" << pg.first << "] ";
        std::copy(pg.second.begin(), pg.second.end(),
                std::ostream_iterator<rabbitxx::VertexDescriptor>(std::cout, ", "));
        std::cout << "\n";
    }

    REQUIRE(exp_local_pgmap == lpgs);
}

TEST_CASE("[ech]", "Find local Process Groups of Synchronization Events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/edge_case_hard/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    //auto lpgs = rabbitxx::make_local_pgmap(*graph);


    rabbitxx::pg_map_t<rabbitxx::VertexDescriptor> exp_local_pgmap {
        { 21, { 0, 1 } },
        { 22, { 2, 3 } },
        { 26, { 0, 2 } },
        { 29, { 0, 1 } },
        { 30, { 2, 3 } },
    };

    REQUIRE(exp_local_pgmap == lpgs);
}

TEST_CASE("[trace-own-advanced6]", "Find local Process Groups of Synchronization Events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-own_trace6_advanced/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    //auto lpgs = rabbitxx::make_local_pgmap(*graph);


    rabbitxx::pg_map_t<rabbitxx::VertexDescriptor> exp_local_pgmap {
        { 25, { 4, 5 } },
        { 29, { 0, 1 } },
        { 30, { 2, 3 } },
        { 36, { 2, 3 } },
        { 39, { 4, 5 } },
        { 42, { 1, 4 } },
        { 47, { 0, 1 } }
    };

    REQUIRE(exp_local_pgmap == lpgs);
}

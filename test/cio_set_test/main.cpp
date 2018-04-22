//#define CATCH_CONFIG_MAIN
//#include "catch.hpp"

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

/**
 * madbench2
 * Process 0
 * Set 1
 * start: 1
 * end: 11
 * events 10, 9, 8, 7, 5, 6
 *
 */

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    // create graph
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    // find concurrent I/O-Sets
    auto io_sets = rabbitxx::find_cio_sets(graph);

    for (const auto& set : io_sets)
    {
        std::cout << set;
        std::cout << "\n\n";
    }

    return 0;
}

//#define CATCH_CONFIG_MAIN
//#include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <boost/mpi.hpp>

#include <iostream>

using rabbitxx::logging;

//TEST_CASE("create empty cio_set", "[constructor]")
//{

//}

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    // create graph
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    // find concurrent I/O-Sets
    auto io_sets = rabbitxx::collect_concurrent_io_sets(*graph.get());

    std::cout << "Print Sets per Process!\n";
    for (const auto psets : *io_sets) {
        std::cout << "[Process] " << psets.first << " has " << psets.second.size() << " sets!\n";
        for (const auto set : psets.second) {
            std::cout << set;
        }
        std::cout << "\n";
    }

    return 0;
}

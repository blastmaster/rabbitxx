// #define CATCH_CONFIG_MAIN
// #include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <boost/mpi.hpp>

// TEST_CASE("generic graph builder", "[graph_builder<SimpleGraph>]")
// {
int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;
    static const std::string trace_file = "/home/soeste/code/otf2cpp/traces/hacc_io/scorep-20170821_1239_11843901293182859/traces.otf2";

    using rabbitxx::trace::OTF2_Io_Graph_Builder;
    auto graph = rabbitxx::make_graph<OTF2_Io_Graph_Builder>(trace_file, world);
    
    return 0;
}

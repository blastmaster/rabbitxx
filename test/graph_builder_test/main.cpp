#include <rabbitxx/trace/graph_builder.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <otf2xx/otf2.hpp>

#include <boost/mpi.hpp>

#include <iostream>

using rabbitxx::logging;

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        std::cerr << "usage: ./" << argv[0] << " <input-trace>" << std::endl;
        env.abort(1);
        return 1;
    }

    otf2::reader::reader rdr(argv[1]);
    auto num_locations = rdr.num_locations();

    if (world.rank() == 0) {
        logging::debug() << "Number of locations: " << num_locations
                        << " world size: " << world.size();
    }

    rabbitxx::trace::graph_builder<rabbitxx::Graph> graph_builder(world);
    rdr.set_callback(graph_builder);

    rdr.read_definitions();
    world.barrier();

    rdr.read_events();

    world.barrier();
    auto& g = graph_builder.graph();

    auto n_vertices = g.num_vertices();
    logging::debug() << "Num vertices: " << n_vertices;

    return 0;
}

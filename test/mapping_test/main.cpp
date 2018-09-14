#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <boost/mpi.hpp>

using rabbitxx::logging;

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace>";
        env.abort(1);
        return 1;
    }

    otf2::reader::reader trc_reader(argv[1]);
    auto num_locations = trc_reader.num_locations();
    rabbitxx::graph::io_graph_builder builder(world, num_locations);

    trc_reader.set_callback(builder);
    trc_reader.read_definitions();
    world.barrier();
    trc_reader.read_events();
    world.barrier();

    if (world.rank() == 0) {
        const auto& pmap = builder.get_mapping();
        dump_mapping(pmap);
    }


    return 0;
}

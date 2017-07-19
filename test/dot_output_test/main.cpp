#include <rabbitxx/graph.hpp>
#include <rabbitxx/trace/simple_graph_builder.hpp>
#include <rabbitxx/log.hpp>

#include <otf2xx/otf2.hpp>

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

    otf2::reader::reader rdr(argv[1]);
    auto num_locations = rdr.num_locations();

    rabbitxx::trace::simple_graph_builder<rabbitxx::SimpleGraph> gb(world, num_locations);
    rdr.set_callback(gb);
    rdr.read_definitions();
    world.barrier();
    rdr.read_events();
    world.barrier();

    auto& g = gb.graph();
    const auto nv = g.num_vertices();
    logging::debug() << "num vertices: :" << nv;
    const std::string filename {"test.dot"};

    rabbitxx::write_graph_to_dot(g, filename);

    return 0;
}

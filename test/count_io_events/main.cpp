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

    auto g = rabbitxx::make_graph<rabbitxx::trace::OTF2_Io_Graph_Builder>(argv[1], world);
    const auto vip = g->vertices();
    if (world.rank() == 0) {
        int count = 0;
        std::for_each(vip.first, vip.second, [&g, &count](const auto& d) {
                if (g->operator[](d).type == rabbitxx::vertex_kind::io_event) {
                    auto vertex = g->operator[](d).property;
                    logging::debug() << "vertex: " << vertex;
                    ++count;
                }
        });
        logging::debug() << count << " I/O events";
    }

    return 0;
}

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
    const auto nv = g->num_vertices();
    logging::debug() << "num vertices: :" << nv;
    const std::string filename {"test.dot"};

    rabbitxx::write_graph_to_dot(*(g->get()), filename);

    return 0;
}

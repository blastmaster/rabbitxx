#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <boost/mpi.hpp>

using rabbitxx::logging;

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;
    std::string filename {"test.dot"};

    switch (argc)
    {
        case 2:
          // TODO: take dir name of the parent dir of the trace as basename
          break;
        case 3:
            filename.replace(filename.begin(), filename.end(), argv[2]);
            logging::debug() << "Output filename set to: " << filename;
            break;
        default:
            logging::fatal() << "usage: ./" << argv[0] << " <input-trace>" << " [outfile]";
            env.abort(1);
            return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);

    logging::debug() << "Write graph to: " << filename;
    auto cio_sets = gather_concurrent_io_sets(*g.get());
    rabbitxx::write_graph_to_dot(*(g->get()), filename, cio_sets);

    return 0;
}

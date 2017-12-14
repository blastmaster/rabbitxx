#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>


using rabbitxx::logging;

int main(int argc, char** argv)
{
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
            return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);

    logging::debug() << "Write graph to: " << filename;
    auto cio_sets = find_cio_sets(*g.get());
    rabbitxx::write_graph_to_dot(*(g->get()), filename, cio_sets);

    return 0;
}

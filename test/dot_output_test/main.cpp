#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace>";
        env.abort(1);
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    const std::string filename {"test.dot"};
    rabbitxx::write_graph_to_dot(*(g->get()), filename);

    return 0;
}

#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

void print_graph_properties(const rabbitxx::IoGraph& graph)
{
    std::cout << graph.graph_properties();
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace> ";
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    print_graph_properties(g);

    return 0;
}

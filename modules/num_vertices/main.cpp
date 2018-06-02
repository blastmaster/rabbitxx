#include <rabbitxx/graph.hpp>
#include <iostream>

using namespace rabbitxx;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trc_file);
    auto n_v = graph.num_vertices();
    auto edges = graph.num_edges();

    std::cout << "num vertices: " << n_v << "\n";
    std::cout << "num edges: " << edges << "\n";

    return EXIT_SUCCESS;
}

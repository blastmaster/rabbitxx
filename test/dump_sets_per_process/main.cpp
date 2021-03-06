#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    // create graph
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    // find concurrent I/O-Sets per process
    auto io_sets = rabbitxx::cio_sets_per_process(graph);

    for (const auto& proc_sets : io_sets)
    {
        std::cout << "Process [" << proc_sets.first << "]\n";
        std::copy(proc_sets.second.begin(), proc_sets.second.end(),
                std::ostream_iterator<rabbitxx::set_t<rabbitxx::VertexDescriptor>>(std::cout, ", "));
        std::cout << "\n\n";
    }

    return 0;
}

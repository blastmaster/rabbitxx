#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
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
    using vertex_descriptor = typename decltype(graph)::element_type::vertex_descriptor;
    // find concurrent I/O-Sets per process
    auto io_sets = rabbitxx::cio_sets_per_process(*graph.get());

    for (const auto& proc_sets : io_sets)
    {
        std::cout << "Process [" << proc_sets.first << "]\n";
        std::copy(proc_sets.second.begin(), proc_sets.second.end(),
                std::ostream_iterator<rabbitxx::set_t<vertex_descriptor>>(std::cout, ", "));
        std::cout << "\n\n";
    }

    return 0;
}

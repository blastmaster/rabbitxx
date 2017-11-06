#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <boost/mpi.hpp>

using rabbitxx::logging;

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    //rabbitxx::collect_concurrent_sets(*graph.get());
    //rabbitxx::type_printer<typename decltype(graph)::element_type::vertex_descriptor> tp;

    //std::vector<rabbitxx::CIO_Set<
        //rabbitxx::otf2_trace_event,
        //typename decltype(graph)::element_type::vertex_descriptor>> setv;

    auto io_sets = rabbitxx::collect_concurrent_io_sets(*graph.get());
    logging::debug() << "\n\n";
    //FIXME remove empty sets
    io_sets->erase(std::remove_if(
                io_sets->begin(),
                io_sets->end(),
                [](const auto& set) { return set.empty(); }),
                io_sets->end());


    for (const auto& set : *io_sets) {
        std::cout << set;
        std::cout << "Events:\n";
        for (auto evt : set) {
            std::cout << "@" << graph->operator[](evt).id()
                << " Name: " << graph->operator[](evt).name() << "\n";
        }
    }

    return 0;
}

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

    // create graph
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    // find concurrent I/O-Sets
    auto io_sets = rabbitxx::collect_concurrent_io_sets(*graph.get());
    std::cout << "================================================================================\n";
    const auto n_sets_before_filter = io_sets->size();
    //FIXME remove empty sets
    io_sets->erase(std::remove_if(
                io_sets->begin(),
                io_sets->end(),
                [](const auto& set){ return set.empty(); }),
                io_sets->end());
    const auto n_sets_after_filter = io_sets->size();

    std::cout << "CIO_Sets before filter: " << n_sets_before_filter << "\n";
    std::cout << "CIO_Sets after filter: " << n_sets_after_filter << "\n";
    std::cout << "\n";

    int cnt = 0;
    for (const auto& set : *io_sets) {
        std::cout << "Found set number: " << ++cnt << "\n";
        std::cout << (set.is_closed() ? "Set is closed" : "Set is NOT closed") << "\n";
        std::cout << "Start-Event: @" << graph->operator[](set.start_event()).id() << " " << graph->operator[](set.start_event()).name() << "\n";
        std::cout << "End-Event: @" << graph->operator[](set.end_event().value()).id() << " " << graph->operator[](set.end_event().value()).name() << "\n";
        std::cout << "Number of Events: " << set.size() << "\n";
        std::cout << "Events:\n";
        for (auto evt : set) {
            std::cout << "@" << graph->operator[](evt).id()
                << " Name: " << graph->operator[](evt).name() << "\n";
        }
        std::cout << "\n";
        std::cout << "================================================================================\n";
    }

    return 0;
}

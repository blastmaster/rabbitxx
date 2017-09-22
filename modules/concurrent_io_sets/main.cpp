#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>
#include <boost/mpi.hpp>

#include <iostream>
#include <algorithm>
#include <string>

using rabbitxx::logging;

template<typename Graph>
void collect_concurrent_sets(Graph& graph)
{
    auto vertices = graph.vertices();
    auto it = std::find(vertices.first, vertices.second,
                        [&graph](const typename Graph::vertex_descriptor& vd)
                        {
                            return true;
                        });
}


template<typename Graph>
std::vector<int>
get_out_degrees(Graph& graph)
{
    using descriptor = typename Graph::vertex_descriptor;
    std::vector<int> out_degrees(graph.num_vertices());
    const auto vertices = graph.vertices();
    std::transform(vertices.first, vertices.second, std::begin(out_degrees),
                [&graph](const descriptor& vd)
                {
                    return static_cast<int>(boost::out_degree(vd, *(graph.get())));
                }
    );
    return out_degrees;
}


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

    logging::debug() << "Try to read first vertex";
    std::cout << graph->operator[](0) << std::endl;

    const auto out_degrees = get_out_degrees(*graph.get());
    for (const auto& out_d : out_degrees)
    {
        std::cout << out_d << std::endl;
    }

    return 0;
}

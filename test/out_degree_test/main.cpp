#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <algorithm>
#include <iostream>
#include <string>

using rabbitxx::logging;

template<typename Graph>
std::vector<std::pair<int, std::string>>
get_out_degrees(Graph& graph)
{
    using descriptor = typename Graph::vertex_descriptor;
    std::vector<std::pair<int, std::string>> out_degrees(graph.num_vertices());
    const auto vertices = graph.vertices();
    std::transform(vertices.first, vertices.second, std::begin(out_degrees),
                [&graph](const descriptor& vd)
                {
                    return std::make_pair(
                        static_cast<int>(boost::out_degree(vd, *(graph.get()))),
                        graph[vd].name()
                    );
                }
    );
    return out_degrees;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);

    logging::debug() << "Try to read first vertex";
    std::cout << graph[0] << std::endl;

    const auto out_degrees = get_out_degrees(graph);
    for (const auto& out_d : out_degrees)
    {
        std::cout << out_d.second << " " << out_d.first << std::endl;
    }

    return 0;
}

#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

template<typename Graph>
auto
get_sync_events(Graph& g)
{
    const auto vip = g.vertices();
    std::vector<typename Graph::vertex_descriptor> sync_events;
    std::copy_if(vip.first, vip.second, std::back_inserter(sync_events),
            [&g](const typename Graph::vertex_descriptor& vd) {
                return g[vd].type == rabbitxx::vertex_kind::sync_event;
            });
    return sync_events;
}

template<typename Graph>
auto
get_io_events(Graph& g)
{
    const auto vip = g.vertices();
    std::vector<typename Graph::vertex_descriptor> io_events;
    std::copy_if(vip.first, vip.second, std::back_inserter(io_events),
                [&g](const typename Graph::vertex_descriptor& vd) {
                    return g[vd].type == rabbitxx::vertex_kind::io_event;
                });
    return io_events;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace> [<filter=[io|sync]>]";
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    auto print = [&g](const auto& evt_vec)
    {
        for (const auto evt : evt_vec) {
            std::cout << g->operator[](evt) << std::endl;
        }
    };

    if (argc == 3) {
        std::string filter {argv[2]};
        if (filter == "io") {
            auto io_evts = get_io_events(*g);
            print(io_evts);
        }
        else if (filter == "sync") {
            auto sync_evts = get_sync_events(*g);
            print(sync_evts);
        }
    }

    return 0;
}

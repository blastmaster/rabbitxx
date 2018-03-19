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

template<typename DurationT=otf2::chrono::nanoseconds>
void print_info_as(const rabbitxx::app_info& info)
{
    std::cout << "total time: " << otf2::chrono::duration_cast<DurationT>(info.total_time) << "\n";
    std::cout << "total file io time: " << otf2::chrono::duration_cast<DurationT>(info.io_time) << "\n";
    std::cout << "total file io metadata time: " << otf2::chrono::duration_cast<DurationT>(info.io_metadata_time) << "\n";
    std::cout << "first event time: " << info.first_event_time << "\n";
    std::cout << "last event time: " << info.last_event_time << "\n";

    std::cout << "first event time duration: " 
        << otf2::chrono::duration_cast<DurationT>(info.first_event_time.time_since_epoch()) << "\n";
    std::cout << "last event time duration: " << otf2::chrono::duration_cast<DurationT>(info.last_event_time.time_since_epoch()) << "\n";

    std::cout << "Clock properties:\n"
        << "ticks per second: " << info.clock_props.ticks_per_second().count() << "\n"
        << "start time: " << info.clock_props.start_time().count() << "\n"
        << "length: " << info.clock_props.length().count() << "\n";
}

void print_graph_properties(const rabbitxx::IoGraph& graph)
{
    auto info = graph.graph_properties();
    print_info_as(info);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace> [<filter=[io|sync|all]>]";
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
        else if (filter == "all") {
            auto vip = g->vertices();
            for (auto it = vip.first; it != vip.second; ++it)
            {
                auto trc_evt = g->operator[](*it);
                std::cout << trc_evt << "\n";
            }
            print_graph_properties(*g);
        }
    }

    return 0;
}

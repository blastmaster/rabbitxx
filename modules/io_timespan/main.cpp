#include <rabbitxx/graph.hpp>
#include <rabbitxx/utils.hpp>

using namespace rabbitxx;

/**
 * TODO:
 * Gesamtlaufzeit scheint, clock_properties.length() / clock_properties.ticks_per_second() zu sein.
 *
 * Probiere,
 * enter und leave intervalle in millisekunden darzustellen und mit vampir
 * vergleichen.
 *
 */

template<typename DurationT=otf2::chrono::microseconds>
void print_total_duration(const app_info& info)
{

    std::cout << "start time "
        << std::chrono::duration_cast<DurationT>(info.first_event_time.time_since_epoch()) << "\n";
    std::cout << "end time "
        << std::chrono::duration_cast<DurationT>(info.last_event_time.time_since_epoch()) << "\n";
}

//TODO: dump as csv!
void dump_table(const otf2_trace_event& evt)
{
    if (evt.type == vertex_kind::io_event)
    {
        //std::cout << evt.id() << ", " << evt.duration.enter.time_since_epoch().count() << ", " << evt.duration.leave.time_since_epoch().count() << "\n";
        //std::cout << evt.id() << " " << evt.duration.enter << " " << evt.duration.leave << "\n";
        if (evt.duration.enter == otf2::chrono::armageddon() || evt.duration.leave == otf2::chrono::armageddon()) {
            return;
        }
        auto kind = boost::get<rabbitxx::io_event_property>(evt.property).kind; 
        std::cout << evt.id() << " " 
            << duration_to_string<otf2::chrono::microseconds>(evt.duration.enter.time_since_epoch()) << " "
            << duration_to_string<otf2::chrono::microseconds>(evt.duration.leave.time_since_epoch()) << " "
            << evt.name() << " " << kind << "\n";
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "usage: ./" << argv[0] << " <input-trace>";
        return 1;
    }

    auto g = make_graph<graph::OTF2_Io_Graph_Builder>(argv[1]);
    auto vip = g.vertices();
    print_total_duration(g.graph_properties());
    for (auto it = vip.first; it != vip.second; ++it)
    {
        auto trc_evt = g[*it];
        dump_table(trc_evt);
    }

    return 0;
}

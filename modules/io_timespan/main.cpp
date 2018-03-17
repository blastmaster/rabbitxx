#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

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

/**
 * TODO:
 * Gesamtlaufzeit scheint, clock_properties.length() / clock_properties.ticks_per_second() zu sein.
 *
 * Probiere,
 * enter und leave intervalle in millisekunden darzustellen und mit vampir
 * vergleichen.
 *
 */

template<typename DurationT=otf2::chrono::nanoseconds>
void print_total_duration(const rabbitxx::app_info& info)
{

    std::cout << "start time "
        << otf2::chrono::duration_cast<DurationT>(info.first_event_time.time_since_epoch()) << "\n";
    std::cout << "end time "
        << otf2::chrono::duration_cast<DurationT>(info.last_event_time.time_since_epoch()) << "\n";
}

template<typename DurationT=otf2::chrono::nanoseconds>
std::string duration_to_string(const otf2::chrono::duration& dur)
{
    std::stringstream ss;
    ss << otf2::chrono::duration_cast<DurationT>(dur);
    return ss.str();
}

void print_graph_properties(const rabbitxx::IoGraph& graph)
{
    auto info = graph.graph_properties();
    print_info_as<otf2::chrono::nanoseconds>(info);
}

void dump_table(const rabbitxx::otf2_trace_event& evt)
{
    if (evt.type == rabbitxx::vertex_kind::io_event)
    {
        //std::cout << "Proc ID: " << evt.id() << "\n";
        //std::cout << "[ " << evt.duration.enter << ", " << evt.duration.leave << " ]\n";
        //std::cout << evt.id() << ", " << evt.duration.enter.time_since_epoch().count() << ", " << evt.duration.leave.time_since_epoch().count() << "\n";
        //std::cout << evt.id() << " " << evt.duration.enter << " " << evt.duration.leave << "\n";
        if (evt.duration.enter == otf2::chrono::armageddon() || evt.duration.leave == otf2::chrono::armageddon()) {
            return;
        }
        std::cout << evt.id() << " " 
            << duration_to_string<otf2::chrono::nanoseconds>(evt.duration.enter.time_since_epoch()) << " "
            << duration_to_string<otf2::chrono::nanoseconds>(evt.duration.leave.time_since_epoch()) << " "
            << evt.name() << "\n";
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace>";
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    auto vip = g->vertices();
    print_total_duration(g->graph_properties());
    for (auto it = vip.first; it != vip.second; ++it)
    {
        auto trc_evt = g->operator[](*it);
        dump_table(trc_evt);
    }
    //print_graph_properties(*g);

    return 0;
}

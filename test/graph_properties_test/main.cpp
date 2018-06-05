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

void print_graph_properties(const rabbitxx::IoGraph& graph)
{
    auto info = graph.graph_properties();
    print_info_as(info);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace> ";
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    print_graph_properties(g);

    return 0;
}

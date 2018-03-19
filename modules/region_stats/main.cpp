#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

void dump_regions(rabbitxx::otf2_trace_event& evt)
{
    if (evt.type != rabbitxx::vertex_kind::io_event) {
        return;
    }
    auto io_evt = boost::get<rabbitxx::io_event_property>(evt.property);
    std::cout << io_evt.region_name << "\n";
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace>";
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    auto vip = g->vertices();
    for (auto it = vip.first; it != vip.second; ++it)
    {
        auto trc_evt = g->operator[](*it);
        dump_regions(trc_evt);
    }

    return 0;
}

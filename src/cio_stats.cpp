#include <rabbitxx/cio_stats.hpp>

namespace rabbitxx
{

std::map<std::string, std::vector<VertexDescriptor>>
file_map(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<VertexDescriptor>> f_map;
    for (VertexDescriptor vd : cio_set)
    {
        auto io_evt = get_io_property(graph, vd);
        f_map[io_evt.filename].push_back(vd);
    }
    return f_map;
}

otf2::chrono::microseconds
get_set_duration(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    auto start_vd = cio_set.start_event();
    auto end_vd = cio_set.end_event().value();
    auto ts_start = graph[start_vd].timestamp();
    auto ts_end = graph[end_vd].timestamp();
    if (graph[start_vd].type == vertex_kind::synthetic)
    {
        //take timestamp of first event in set
        //std::cerr << "set start event ts from: " << ts_start;
        ts_start = graph[*cio_set.begin()].timestamp();
        //std::cerr << " to " << ts_start << "\n";
    }
    if (graph[end_vd].type == vertex_kind::synthetic)
    {
        //std::cerr << "set end event ts from: " << ts_end;
        ts_end = graph[*std::prev(cio_set.end())].timestamp();
        //std::cerr << " to " << ts_end << "\n";
    }
    assert(ts_end > ts_start);
    auto dur_diff = ts_end - ts_start;
    return otf2::chrono::duration_cast<otf2::chrono::microseconds>(dur_diff);
}

} //namespace rabbitxx

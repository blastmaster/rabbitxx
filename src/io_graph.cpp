#include <rabbitxx/graph/io_graph.hpp>


namespace rabbitxx {


io_event_property 
get_io_property(const IoGraph& graph, const VertexDescriptor vd)
{
    return boost::get<io_event_property>(graph[vd].property);
}

sync_event_property
get_sync_property(const IoGraph& graph, const VertexDescriptor vd)
{
    return boost::get<sync_event_property>(graph[vd].property);
}

VertexDescriptor
find_root(const IoGraph& graph)
{
    const auto vertices = graph.vertices();
    const auto root = std::find_if(
        vertices.first, vertices.second, [&graph](const VertexDescriptor& vd) {
            return graph[vd].type == vertex_kind::synthetic;
        });
    return *root;
}


std::uint64_t
num_procs(const IoGraph& graph) noexcept
{
    const auto root = find_root(graph);
    return static_cast<std::uint64_t>(graph.out_degree(root));
}

std::vector<std::uint64_t>
procs_in_sync_involved(const sync_event_property& sevt)
{
    if (sevt.comm_kind == sync_event_kind::collective)
    {
        const auto coll_evt = boost::get<collective>(sevt.op_data);
        return coll_evt.members();
    }
    if (sevt.comm_kind == sync_event_kind::p2p)
    {
        const auto p2p_evt = boost::get<peer2peer>(sevt.op_data);
        return { sevt.proc_id, p2p_evt.remote_process() };
    }
    return {};
}

std::uint64_t
num_procs_in_sync_involved(const sync_event_property& sevt)
{
    return procs_in_sync_involved(sevt).size();
}

std::vector<VertexDescriptor>
get_events_by_kind(const IoGraph& graph, const std::vector<vertex_kind>& kinds)
{
    using vertex_descriptor = VertexDescriptor;
    const auto vp = graph.vertices();
    std::vector<vertex_descriptor> events;
    std::copy_if(vp.first, vp.second, std::back_inserter(events),
        [&kinds, &graph](const vertex_descriptor& vd) {
            return std::any_of(kinds.begin(), kinds.end(), [&vd, &graph](const vertex_kind& kind) {
                return graph[vd].type == kind;

            });
        });
    return events;
}

std::vector<VertexDescriptor>
get_io_events_by_kind(const IoGraph& graph, const std::vector<io_event_kind>& kinds)
{
    const auto io_events = get_events_by_kind(graph, { vertex_kind::io_event });
    std::vector<VertexDescriptor> io_evts_by_kind;
    std::copy_if(io_events.begin(), io_events.end(), std::back_inserter(io_evts_by_kind),
            [&kinds, &graph](const VertexDescriptor& vd) {
                return std::any_of(kinds.begin(), kinds.end(),
                        [&vd, &graph](const io_event_kind& kind) {
                            return boost::get<io_event_property>(graph[vd].property).kind = kind;
                        });
                });
    return io_evts_by_kind;
}

sync_scope
classify_sync(const IoGraph& g, const sync_event_property& sevt)
{
    const auto np = num_procs(g);
    const auto inv = num_procs_in_sync_involved(sevt);
    return np == inv ? sync_scope::Global : sync_scope::Local;
}

sync_scope
classify_sync(const IoGraph& g, const VertexDescriptor& v)
{
    if (g[v].type == vertex_kind::synthetic)
    {
        return sync_scope::Global;
    }
    const auto& sync_evt_p = get_sync_property(g, v);
    return classify_sync(g, sync_evt_p);
}

} // namespace rabbitxx

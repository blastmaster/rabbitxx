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
    //TODO throws if event is a synthetic event
    assert(vd != IoGraph::null_vertex());
    return boost::get<sync_event_property>(graph[vd].property);
}

// member function? of graph
// TODO this follows the assumption that the first synthetic vertex is the root.
// Alternatively we can also return `graph.vertices()[0]`, which is much simpler.
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
    return graph.graph_properties().num_locations;
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

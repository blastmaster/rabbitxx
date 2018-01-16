#ifndef RABBITXX_GRAPH_SIMPLE_GRAPH_HPP
#define RABBITXX_GRAPH_SIMPLE_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>

#include <rabbitxx/graph/bgl_base_graph.hpp>
#include <rabbitxx/trace/otf2_trace_event.hpp>

namespace rabbitxx {

namespace graph { namespace detail {

    using simple_io_graph_impl = boost::adjacency_list<
                                                    boost::vecS, // store out-edges of vertex in std::vector
                                                    boost::vecS, // store vertices in a std::vector
                                                    //boost::directedS, // the graph is directed
                                                    boost::bidirectionalS, // the graph is directed
                                                    otf2_trace_event>; // use otf2_trace_event as vertex property
}} // namespace graph::detail

using SimpleIoGraph = graph::graph<graph::detail::simple_io_graph_impl>;

// API
/**
 * @brief Get all in-going synchronization-events of a given vertex.
 *
 * @param v: The vertex descriptor of the synchronization-event.
 * @param g: A Reference to the Graph.
 *
 * @return Retruns a vector containing the vertex descriptors of all in-going
 * synchronization events from a given vertex v.
 */
template <typename Vertex, typename Graph>
std::vector<Vertex>
get_in_going_syncs(Vertex v, Graph& g)
{
    const auto in_edge_r = boost::in_edges(v, g);
    std::vector<Vertex> results;
    for (auto edge_it = in_edge_r.first; edge_it != in_edge_r.second; ++edge_it)
    {
        const auto src_v = source(*edge_it, g);
        if (vertex_kind::sync_event == g[src_v].type)
        {
            results.push_back(src_v);
        }
    }
    return results;
}

// API
/**
 * @brief Find the root-Event of a given Synchronization Event.
 *
 * @tparam Vertex The vertex-descriptor-type
 * @tparam Graph The graph-type
 * @param v the vertex descriptor of a synchronization-event.
 * @param g a reference to the graph.
 *
 * @return The vertex descriptor of the root-event of the synchronization.
 */
template <typename Vertex, typename Graph>
Vertex
root_of_sync(Vertex v, Graph& g)
{
    assert(vertex_kind::sync_event == g[v].type);
    const auto in_dgr = boost::in_degree(v, g);
    if (in_dgr == 1)
    {
        return v;
    }
    const auto sync_evt = boost::get<sync_event_property>(g[v].property);
    // if p2p event get remote proc
    if (sync_evt.comm_kind == sync_event_kind::p2p)
    {
        const auto p2p_evt = boost::get<peer2peer>(sync_evt.op_data);
        const auto r_proc = p2p_evt.remote_process();
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, g);
        for (const auto s : in_syncs)
        {
            const auto id = g[s].id();
            if (id == r_proc)
            { // Test if r_proc is proc_id from root!
                logging::debug() << "process id matches remote proc id!";
                return s;
            }
        }
    } // if collective
    else if (sync_evt.comm_kind == sync_event_kind::collective)
    {
        const auto coll_evt = boost::get<collective>(sync_evt.op_data);
        // check if collective has an explicit root
        if (coll_evt.has_root())
        {
            const auto root_rank = coll_evt.root();
            if (root_rank == g[v].id())
            { // we *are* the root sync event!
                logging::debug() << " v " << v << " root rank " << root_rank;
                return v; // return our own vertex descriptor
            }
        }
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, g);
        for (const auto s : in_syncs)
        {
            const auto in_dgr = boost::in_degree(s, g);
            const auto out_dgr = boost::out_degree(s, g);
            if ((out_dgr >= coll_evt.members().size() - 1) && (in_dgr == 1))
            {
                return s;
            }
        }
    }
    // if we reached the end of function without finding a root vertex. THROw
    logging::fatal() << "Error no root of sync-event could be found. This should never happen!";
    throw -1; // FIXME: Do useful error-handling.
}

// API
/**
 * @brief Find the synthetic root node of a given graph.
 *
 * Root node is the only vertex with a `vertex_kind::synthetic`.
 *
 * @param graph: A reference to the Graph.
 * @return The vertex descriptor of the root node.
 */
template <typename Graph>
typename Graph::vertex_descriptor
find_root(Graph& graph)
{
    const auto vertices = graph.vertices();
    const auto root = std::find_if(
        vertices.first, vertices.second, [&graph](const typename Graph::vertex_descriptor& vd) {
            return graph[vd].type == vertex_kind::synthetic;
        });
    return *root;
}

// API
/**
 * @brief Get the number of processes involved, in a given I/O Graph.
 *
 * Therefore we take the synthetic root vertex and count the out-going edges.
 *
 * @param graph: A reference to the graph.
 * @return The number of processes that have events in the graph.
 */
template <typename Graph>
std::uint64_t
num_procs(Graph& graph) noexcept
{
    const auto root = find_root(graph);
    return static_cast<std::uint64_t>(boost::out_degree(root, *graph.get()));
}

// API
/**
 * @brief Get all processes which are involved in a given synchronization event.
 *
 * Return vector containing all the process id's of the processes participating
 * within a given synchronization event.
 *
 * @param sevt: Reference to a synchronization event.
 * @return Vector of process ids, which are involved in the synchronization.
 * If the sync-event is wether a collective nor a peer2peer event the returned
 * vector is empty.
 */
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

// template<typename Graph, typename VertexDescriptor>
// std::vector<std::uint64_t>
// procs_in_sync_involved(Graph& graph, const VertexDescriptor& vd)
//{
// const auto type = graph[vd].type;
// switch (type)
//{
// case vertex_kind::sync_event:
// const auto s_evt = boost::get<sync_event_property>(graph[vd].property);
// return procs_in_sync_involved(s_evt);
// case vertex_kind::synthetic:
// const auto np = num_procs(graph);
// std::vector<std::uint64_t> all_procs(np);
// std::iota(all_procs.begin(), all_procs.end(), 0);
// return all_procs;
// default:
// logging::fatal() << "ERROR in procs_in_sync_involved, invalid synhronization
// event!";
//}
//}

// API
/**
 * Return the number of processes involved in a given synchronization routine.
 */
std::uint64_t
num_procs_in_sync_involved(const sync_event_property& sevt)
{
    return procs_in_sync_involved(sevt).size();
}

// API
// TODO: unused!
template <typename Graph, typename Vertex, typename Visitor>
void
traverse_adjacent_vertices(Graph& graph, Vertex v, Visitor& vis)
{
    typename Graph::adjacency_iterator adj_begin, adj_end;
    for (std::tie(adj_begin, adj_end) = boost::adjacent_vertices(v, *graph.get());
         adj_begin != adj_end; ++adj_begin)
    {
        vis(graph, *adj_begin);
        traverse_adjacent_vertices(graph, *adj_begin, vis);
    }
}

// API
/**
* Checks if the start event of a set `of_v` is an adjacent vertex of the current
* event `cur_v`.
*/
template <typename Vertex, typename Graph>
bool
is_adjacent_event_of(const Vertex of_v, const Vertex cur_v, const Graph& g)
{
    const auto adjacent_r = boost::adjacent_vertices(cur_v, g);
    return std::any_of(
        adjacent_r.first, adjacent_r.second, [&of_v](const Vertex vd) { return vd == of_v; });
}

// Graph-API
// should maybe a variadic template
template <typename Graph>
std::vector<typename Graph::vertex_descriptor>
get_events_by_kind(Graph& graph, const std::vector<vertex_kind>& kinds)
{
    using vertex_descriptor = typename Graph::vertex_descriptor;
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

} // namespace rabbitxx

#endif /* RABBITXX_GRAPH_SIMPLE_GRAPH_HPP */

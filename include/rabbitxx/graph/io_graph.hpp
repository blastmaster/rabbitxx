#ifndef RABBITXX_IO_GRAPH_HPP
#define RABBITXX_IO_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>

#include <rabbitxx/graph/bgl_base_graph.hpp>
#include <rabbitxx/graph/otf2_io_graph_properties.hpp>

namespace rabbitxx {

namespace graph { namespace detail {

    using io_graph_impl = boost::adjacency_list<
                                                    boost::vecS, // store out-edges of vertex in std::vector
                                                    boost::vecS, // store vertices in a std::vector
                                                    boost::directedS, // the graph is directed
                                                    otf2_trace_event, // use otf2_trace_event as vertex property
                                                    boost::no_property, // no edge property
                                                    app_info>; // use app_info as graph property
}} // namespace graph::detail

using IoGraph = graph::graph<graph::detail::io_graph_impl>;
// try to define the vertex_descriptor type to avoid templates!
// since we dependet on an integral vertex descriptor type.
using VertexDescriptor = typename IoGraph::vertex_descriptor;


io_event_property get_io_property(const IoGraph& graph, const VertexDescriptor vd);

sync_event_property get_sync_property(const IoGraph& graph, const VertexDescriptor vd);

/**
 * @brief Find the synthetic root node of a given graph.
 *
 * Root node is the only vertex with a `vertex_kind::synthetic`.
 *
 * @param graph: A reference to the Graph.
 * @return The vertex descriptor of the root node.
 */
VertexDescriptor find_root(const IoGraph& graph);

/**
 * @brief Get the number of processes involved, in a given I/O Graph.
 *
 * Therefore we take the synthetic root vertex and count the out-going edges.
 *
 * @param graph: A reference to the graph.
 * @return The number of processes that have events in the graph.
 */
std::uint64_t num_procs(const IoGraph& graph) noexcept;

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
std::vector<std::uint64_t> procs_in_sync_involved(const sync_event_property& sevt);

/**
 * @return The number of processes involved in a given synchronization routine.
 */
std::uint64_t num_procs_in_sync_involved(const sync_event_property& sevt);

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

/**
 * @brief Get the `sync_scope` of a synchronization event.
 *
 * @param g: An `IoGraph` object.
 * @param sevt: A `sync_event_property` object.
 *
 * @return The corresponding `sync_scope` of the synchronization event.
 * Either `sync_scope::local` or `sync_scope::global`.
 */
sync_scope
classify_sync(const IoGraph& g, const sync_event_property& sevt);

// What should be returned in the case that `v` is referencing an I/O-Event?
// One option might be to throw an exception.
// Another option might be to introduce a `sync_scope::None` enumeration. On
// which can be checked outside.
sync_scope classify_sync(const IoGraph& g, const VertexDescriptor& v);

} // namespace rabbitxx

#endif /* RABBITXX_IO_GRAPH_HPP */

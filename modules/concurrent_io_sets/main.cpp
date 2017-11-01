#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>
#include <boost/mpi.hpp>

#include <iostream>
#include <algorithm>
#include <string>

using rabbitxx::logging;

/**
 * First try to define a container for concurrent I/O operation sets.
 * Here we use a std::unordered_set as a basis. This might be good for inserting,
 * but there might be problems when searching or some ordering is needed.
 */
#include <unordered_set>
#include <vector>
#include <memory>

/**
 * Utility function
 * Finds the synthetic root node of a given graph.
 * Root node is the only vertex with a `vertex_kind::synthetic`.
 * Returns the vertex_descriptor of the root node.
 */
template<typename Graph>
typename Graph::vertex_descriptor
find_root(Graph& graph)
{
    const auto vertices = graph.vertices();
    const auto root = std::find_if(vertices.first, vertices.second,
            [&graph](const typename Graph::vertex_descriptor& vd) {
                return graph[vd].type == rabbitxx::vertex_kind::synthetic;
            });
    return *root;
}

/**
 * Return vector containing all the process id's of the processes participating
 * within a given synchronization event.
 */
std::vector<std::uint64_t>
procs_in_sync_involved(const rabbitxx::sync_event_property& sevt)
{
    std::uint64_t procs_involved {0};
    if (sevt.comm_kind == rabbitxx::sync_event_kind::collective) {
        const auto coll_evt = boost::get<rabbitxx::collective>(sevt.op_data);
        return coll_evt.members();
    }
    else if (sevt.comm_kind == rabbitxx::sync_event_kind::p2p) {
        const auto p2p_evt = boost::get<rabbitxx::peer2peer>(sevt.op_data);
        return {sevt.proc_id, p2p_evt.remote_process()};
    }
}

/**
 * Return the number of processes involved in a given synchronization routine.
 */
std::uint64_t num_procs_in_sync_involved(const rabbitxx::sync_event_property& sevt)
{
    return procs_in_sync_involved(sevt).size();
}

template<typename Graph, typename Vertex, typename Visitor>
void traverse_adjacent_vertices(Graph& graph, Vertex v, Visitor& vis)
{
    typename Graph::adjacency_iterator adj_begin, adj_end;
    for (std::tie(adj_begin, adj_end) = boost::adjacent_vertices(v, *graph.get());
         adj_begin != adj_end;
        ++adj_begin)
    {
       vis(graph, *adj_begin);
       traverse_adjacent_vertices(graph, *adj_begin, vis);
    }
}

/**
 * Own Concurrent-I/O-Set Visitor
 * TODO: look at boost::default_{bfs,dfs}_visitor and the generic boost graph
 * visitor concept!
 */
template<typename Graph>
class CIO_Visitor
{
    using cio_set = std::unordered_set<typename Graph::vertex_descriptor>;
    using cio_container = std::vector<cio_set>;
public:

    CIO_Visitor() : current_(std::make_unique<cio_set>()), cio_cnt_()
    {
    }

    template<typename Vertex>
    void operator()(const Graph& graph, const Vertex v)
    {
        if (graph[v].type == rabbitxx::vertex_kind::io_event)
        {
            // add to set, if no current set create a new one
            add_to_set(v);
            logging::debug() << "added vertex: #" << v << " @ "
                << graph[v].id() << " [" << graph[v].name() << "]" << " to current set";
        }
        else if (graph[v].type == rabbitxx::vertex_kind::sync_event)
        {
            const auto sync_event = boost::get<rabbitxx::sync_event_property>(graph[v].property);
            // distinguish between p2p and collective sync event
            logging::debug() << "discovered sync event vertex: #" << v << " @ " << graph[v].id()
                << " [" << graph[v].name() << "] CLOSE SET #" << num_procs_in_sync_involved(sync_event);
        }
    }
private:
    template<typename Vertex>
    void add_to_set(Vertex v)
    {
        assert(current_.get() != nullptr);
        current_->insert(v);
    }

    void close_set()
    {
        assert(current_.get() != nullptr);
        cio_cnt_.push_back(*current_.get());
        current_.reset(new cio_set());
    }

private:
    std::unique_ptr<cio_set> current_;
    cio_container cio_cnt_;
};


template<typename Graph>
void collect_concurrent_sets(Graph& graph)
{
    CIO_Visitor<Graph> vis;
    auto root = find_root(graph);
    logging::debug() << "first vertex at: " << graph[root].id() << " [" << graph[root].name() << "]";
    logging::debug() << "START";
    traverse_adjacent_vertices(graph, root, vis);
}

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    collect_concurrent_sets(*graph.get());

    return 0;
}

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
            logging::debug() << "added vertex: #" << v << " to current set";
        }
        else if (graph[v].type == rabbitxx::vertex_kind::sync_event)
        {
            // distinguish between p2p and collective sync event
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


template<typename Graph, typename Vertex, typename Visitor>
void traverse_adjacent_vertices(Graph& graph, Vertex v, Visitor& vis)
{
    typename Graph::adjacency_iterator adj_begin, adj_end;
    for (std::tie(adj_begin, adj_end) = boost::adjacent_vertices(v, *graph.get());
         adj_begin != adj_end;
        ++adj_begin)
    {
       logging::debug() << graph[*adj_begin]; 
       vis(graph, *adj_begin);
       traverse_adjacent_vertices(graph, *adj_begin, vis);
    }
}


template<typename Graph>
void collect_concurrent_sets(Graph& graph)
{
    auto vertices = graph.vertices();
    auto it = std::find_if(vertices.first, vertices.second,
                        [&graph](const typename Graph::vertex_descriptor& vd)
                        {
                            if (graph[vd].type == rabbitxx::vertex_kind::sync_event)
                            {
                               return true;
                            }
                            return false;
                        });
    CIO_Visitor<Graph> vis;
    logging::debug() << "adjacent vertices:\n";
    //traverse_adjacent_vertices(graph, *it, vis);
    //                                start at beginnging!?!?
    traverse_adjacent_vertices(graph, vertex(0, *graph.get()), vis);
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

    logging::debug() << "Try to read first vertex";
    std::cout << graph->operator[](0) << std::endl;

    collect_concurrent_sets(*graph.get());

    return 0;
}

#include <boost/graph/depth_first_search.hpp>

#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <boost/mpi.hpp>

#include <iostream>

using rabbitxx::logging;

class dfs_print_visitor : public boost::default_dfs_visitor
{
public:
    template<typename Vertex, typename Graph>
    void discover_vertex(Vertex v, const Graph& g) const
    {
        if (g[v].type == rabbitxx::vertex_kind::sync_event)
        {
            auto vertex = boost::get<rabbitxx::sync_event_property>(g[v].property);
            std::cout << "Sync Event Descriptor #" << v << " @ " << vertex.proc_id << " Name: " << vertex.region_name << "\n";
        }
        else if (g[v].type == rabbitxx::vertex_kind::io_event)
        {
            auto vertex = boost::get<rabbitxx::io_event_property>(g[v].property);
            std::cout << "I/O Event Descriptor #" << v << " @ " << vertex.proc_id << " Name: " << vertex.region_name << "\n";
        }
    }
};

class dfs_test_visitor : public boost::default_dfs_visitor
{
    public:
        template<typename Vertex, typename Graph>
        void initialize_vertex(Vertex v, const Graph& g) const
        {
            logging::debug() << "initialize vertex #" << v << " @ " << g[v].id() << " [" << g[v].name() << "]";
        }

        template<typename Vertex, typename Graph>
        void start_vertex(Vertex v, const Graph& g) const
        {
            logging::debug() << "start vertex #" << v << " @ " << g[v].id() << " [" << g[v].name() << "]";
        }

        template<typename Vertex, typename Graph>
        void discover_vertex(Vertex v, const Graph& g) const
        {
            logging::debug() << "discover vertex #" << v << " @ " << g[v].id() << " [" << g[v].name() << "]";
        }

        template<typename Edge, typename Graph>
        void examine_edge(Edge e, const Graph& g) const
        {
            const auto src_vd = source(e, g);
            const auto trg_vd = target(e, g);
            logging::debug() << "examine edge from vertex #" << src_vd << " @ " << g[src_vd].id()
                << " [" << g[src_vd].name() << "] -> to vertex #" << trg_vd << " @ " << g[trg_vd].id()
                << " [" << g[trg_vd].name() << "]";
        }

        template<typename Edge, typename Graph>
        void tree_edge(Edge e, const Graph& g) const
        {
            logging::debug() << "tree edge: ";
        }

        template<typename Edge, typename Graph>
        void back_edge(Edge e, const Graph& g) const
        {
            logging::debug() << "back_edge: ";
        }

        template<typename Edge, typename Graph>
        void forward_or_cross_edge(Edge e, const Graph& g) const
        {
            logging::debug() << "forward or cross edge: ";
        }

        template<typename Vertex, typename Graph>
        void finish_vertex(Vertex v, const Graph& g) const
        {
            logging::debug() << "finish vertex #" << v << " @ " << g[v].id() << " [" << g[v].name() << "]";
        }
};

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace> ";
        env.abort(1);
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    //dfs_print_visitor vis;
    dfs_test_visitor vis;
    boost::depth_first_search(*g->get(), boost::visitor(vis));

    return 0;
}

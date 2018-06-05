#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <boost/graph/breadth_first_search.hpp>
#include <boost/mpi.hpp>

#include <iostream>

using rabbitxx::logging;

class bfs_print_visitor : public boost::default_bfs_visitor
{
public:
    template<typename Vertex, typename Graph>
    void discover_vertex(Vertex v, const Graph& g) const
    {
        logging::debug() << "VERTEX # " << v << " " << g[v].name() << " @ " << g[v].id();
    }

    template<typename Edge, typename Graph>
    void examine_edge(Edge e, const Graph& g)
    {
        const auto src_v = source(e,g);
        const auto trg_v = target(e,g);

        logging::debug() << "EDGE from " << g[src_v].name() << " @ " << g[src_v].id()
            << " -> " << g[trg_v].name() << " @ " << g[trg_v].id();

    }
};

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        logging::fatal() << "usage ./" << argv[0] << " <input-trace> ";
        env.abort(1);
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    bfs_print_visitor vis;
    boost::breadth_first_search(*g.get(), vertex(0, *g.get()), boost::visitor(vis));

    return 0;
}

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
       std::cout << v << std::endl; 
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
    dfs_print_visitor vis;
    boost::depth_first_search(*g->get(), boost::visitor(vis));

    return 0;
}

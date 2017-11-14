#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <boost/mpi.hpp>

#include <iostream>
#include <cassert>

using rabbitxx::logging;

template<typename Graph>
std::vector<typename Graph::vertex_descriptor>
get_sync_events(Graph& g)
{
    const auto vip = g.vertices();
    std::vector<typename Graph::vertex_descriptor> sync_events;
    std::copy_if(vip.first, vip.second, std::back_inserter(sync_events),
            [&g](const typename Graph::vertex_descriptor& vd) {
                return g[vd].type == rabbitxx::vertex_kind::sync_event;
            });
    return sync_events;
}

template<typename Vertex, typename Graph>
void dump_in_edges(Vertex v, Graph& g)
{
    auto in_edge_r = boost::in_edges(v, g);
    for (auto edge_it = in_edge_r.first; edge_it != in_edge_r.second; ++edge_it)
    {
        const auto src_vd = source(*edge_it, g); // vertex form where the in edge comes
        const auto trg_vd = target(*edge_it, g); // vertex v!
        logging::debug() << "Found in Edges from vertex # " << src_vd << " "
            << g[src_vd].name() << " @ " << g[src_vd].id()
            << " -> # " << trg_vd << " " << g[trg_vd].name() << " @ " << g[trg_vd].id();
    }
}

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        std::cerr << "Error! usage: " << argv[0] << " <trace-file>" << std::endl;
        return 1;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1], world);
    const auto sync_evts = get_sync_events(*g);
    for (const auto svd : sync_evts) {
        const auto root_evt = rabbitxx::root_of_sync(svd, *g->get());
        logging::debug() << "Root of Sync Event # "  << svd << " @ "
            << g->operator[](svd).id() << " " << g->operator[](svd).name() << " Is: "
            << root_evt << " @ " << g->operator[](root_evt).id() << " " << g->operator[](root_evt).name();
    }

    return 0;
}

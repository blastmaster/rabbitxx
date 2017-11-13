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

template<typename Vertex, typename Graph>
std::vector<Vertex>
get_in_going_syncs(Vertex v, Graph& g)
{
    const auto in_edge_r = boost::in_edges(v, g);
    std::vector<Vertex> results;
    for (auto edge_it = in_edge_r.first; edge_it != in_edge_r.second; ++edge_it) {
        const auto src_v = source(*edge_it, g);
        if (rabbitxx::vertex_kind::sync_event == g[src_v].type) {
            results.push_back(src_v);
        }
    }
    return results;
}

template<typename Vertex, typename Graph>
Vertex root_of_sync(Vertex v, Graph& g)
{
    assert(rabbitxx::vertex_kind::sync_event == g[v].type);
    const auto in_dgr = boost::in_degree(v, *g.get());
    if (in_dgr == 1) {
        return v;
    }
    const auto sync_evt = boost::get<rabbitxx::sync_event_property>(g[v].property);
    // if p2p event get remote proc
    if (sync_evt.comm_kind == rabbitxx::sync_event_kind::p2p) {
        const auto p2p_evt = boost::get<rabbitxx::peer2peer>(sync_evt.op_data);
        const auto r_proc = p2p_evt.remote_process();
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, *g.get());
        for (const auto s : in_syncs) {
            const auto id = g[s].id();
            if (id == r_proc) { // Test if r_proc is proc_id from root!
                logging::debug() << "process id matches remote proc id!";
                return s;
            }
        }
    } // if collective
    else if (sync_evt.comm_kind == rabbitxx::sync_event_kind::collective) {
        const auto coll_evt = boost::get<rabbitxx::collective>(sync_evt.op_data);
        // check if collective has an explicit root
        if (coll_evt.has_root()) {
            const auto root_rank = coll_evt.root();
            if (root_rank == g[v].id()) { // we *are* the root sync event!
                logging::debug() << " v " << v << " root rank " << root_rank;
                return v; // return our own vertex descriptor
            }
        }
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, *g.get());
        for (const auto s : in_syncs) {
            const auto in_dgr = boost::in_degree(s, *g.get());
            const auto out_dgr = boost::out_degree(s, *g.get());
            if ((out_dgr >= coll_evt.members().size() - 1) && (in_dgr == 1)) {
                return s;
            }
        }
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
        const auto root_evt = root_of_sync(svd, *g);
        logging::debug() << "Root of Sync Event # "  << svd << " @ "
            << g->operator[](svd).id() << " " << g->operator[](svd).name() << " Is: "
            << root_evt << " @ " << g->operator[](root_evt).id() << " " << g->operator[](root_evt).name();
    }

    return 0;
}

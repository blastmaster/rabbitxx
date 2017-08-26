#include <rabbitxx/graph.hpp>
#include <rabbitxx/trace/simple_graph_builder.hpp>
#include <rabbitxx/log.hpp>

#include <otf2xx/otf2.hpp>

#include <boost/mpi.hpp>

using rabbitxx::logging;

template<typename Graph>
auto
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

template<typename Graph>
auto
get_io_events(Graph& g)
{
    const auto vip = g.vertices();
    std::vector<typename Graph::vertex_descriptor> io_events;
    std::copy_if(vip.first, vip.second, std::back_inserter(io_events),
                [&g](const typename Graph::vertex_descriptor& vd) {
                    return g[vd].type == rabbitxx::vertex_kind::io_event;
                });
    return io_events;
}

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        logging::fatal() << "usage: ./" << argv[0] << " <input-trace> [<filter=[io|sync]>]";
        env.abort(1);
        return 1;
    }

    otf2::reader::reader rdr(argv[1]);
    auto num_locations = rdr.num_locations();

    rabbitxx::trace::simple_graph_builder<rabbitxx::SimpleGraph> gb(world, num_locations);
    rdr.set_callback(gb);
    rdr.read_definitions();
    world.barrier();
    rdr.read_events();
    world.barrier();

    auto& g = gb.graph();
    auto print = [&g](const auto& evt_vec)
    {
        for (const auto evt : evt_vec) {
            std::cout << g[evt] << std::endl;
        }
    };

    if (argc == 3) {
        std::string filter {argv[2]};
        if (filter == "io") {
            auto io_evts = get_io_events(g);
            print(io_evts);
        }
        else if (filter == "sync") {
            auto sync_evts = get_sync_events(g);
            print(sync_evts);
        }
    }

    return 0;
}

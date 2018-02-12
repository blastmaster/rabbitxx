#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/utils.hpp>
#include <rabbitxx/log.hpp>

#include <string>
#include <iostream>

using rabbitxx::logging;

/**
 * creates-in-dir
 * Find all creates made within an cio-set in the same directory.
 *
 * handle, dir, operation_mode
 * 1. get cio-sets
 * 2. check `create_handle` events and check directory
 * 3. if within same cio-set and within same directory -> store
 *
 */

void
creates_per_dir(rabbitxx::IoGraph& graph, const std::vector<rabbitxx::VertexDescriptor>& create_evts)
//creates_per_dir(rabbitxx::IoGraph& graph, rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set, const std::vector<rabbitxx::VertexDescriptor>& create_evts)
{
    std::map<rabbitxx::fs::path, int> dir_cnt_map;
    for (const auto create_evt : create_evts)
    {
        auto io_evt = boost::get<rabbitxx::io_event_property>(graph[create_evt].property);

        rabbitxx::fs::path p(io_evt.filename);
        p.remove_filename();
        auto it = dir_cnt_map.find(p);
        if (it == dir_cnt_map.end()) {
            dir_cnt_map[p] = 1;
        }
        else {
            dir_cnt_map[p]++;
            std::cout << "in proc " << io_evt.proc_id;
        }
    }

    std::cout << std::endl;
    for (const auto kvp : dir_cnt_map)
    {
        std::cout << kvp.first.c_str() << " " << kvp.second << std::endl;
    }
}

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    // get cio-sets
    auto cio_sets = rabbitxx::find_cio_sets(*graph);
    for (auto& cio_set : cio_sets)
    {
        const auto create_evts = rabbitxx::get_io_events_by_kind(*graph, cio_set, rabbitxx::io_event_kind::create);
        creates_per_dir(*graph, create_evts);
    }

    return EXIT_SUCCESS;
}

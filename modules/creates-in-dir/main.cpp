#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/utils.hpp>
#include <rabbitxx/log.hpp>

#include <string>
#include <iostream>

using rabbitxx::logging;

/**
 * @brief This function should determine if a read-modify-read pattern exists in
 *        the given CIO-Set.
 * @param IoGraph&
 * @param set_t<VertexDescriptor>&
 */
std::map<std::string, std::vector<rabbitxx::VertexDescriptor>>
rmw(const rabbitxx::IoGraph& graph, const rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<rabbitxx::VertexDescriptor>> rw_per_file;
    for (auto vd : cio_set)
    {
        const auto io_evt = boost::get<rabbitxx::io_event_property>(graph[vd].property);
        if (io_evt.kind == rabbitxx::io_event_kind::read ||
                io_evt.kind == rabbitxx::io_event_kind::write)
        {
            const std::string& file = io_evt.filename;
            rw_per_file[file].push_back(vd);
        }
    }
    return rw_per_file;
}

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
std::map<rabbitxx::fs::path, std::vector<rabbitxx::VertexDescriptor>>
creates_per_dir(rabbitxx::IoGraph& graph, const std::vector<rabbitxx::VertexDescriptor>& create_evts)
{
    std::map<rabbitxx::fs::path, std::vector<rabbitxx::VertexDescriptor>> creates_in_dir;
    for (const auto create_evt : create_evts)
    {
        auto io_evt = boost::get<rabbitxx::io_event_property>(graph[create_evt].property);

        rabbitxx::fs::path p(io_evt.filename);
        p.remove_filename();
        creates_in_dir[p].push_back(create_evt);
    }
    return creates_in_dir;
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
    int set_cnt = 0;
    for (auto& cio_set : cio_sets)
    {
        const auto create_evts = rabbitxx::get_io_events_by_kind(*graph, cio_set, rabbitxx::io_event_kind::create);
        auto cpd = creates_per_dir(*graph, create_evts);
        //auto rw = rmw(*graph, cio_set);
        //auto wm = write_functions(*graph, cio_set);
        //print_map_stats(*graph, wm);
        //std::cout << std::endl;
        set_cnt++;
    }

    return EXIT_SUCCESS;
}

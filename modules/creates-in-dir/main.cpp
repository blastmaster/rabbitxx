#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/utils.hpp>

#include <string>
#include <iostream>

using namespace rabbitxx;

/**
 * @brief This function should determine if a read-modify-read pattern exists in
 *        the given CIO-Set.
 * @param IoGraph&
 * @param set_t<VertexDescriptor>&
 */
//std::map<std::string, std::vector<VertexDescriptor>>
//rmw(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
//{
    //std::map<std::string, std::vector<VertexDescriptor>> rw_per_file;
    //for (auto vd : cio_set)
    //{
        //const auto io_evt = boost::get<io_event_property>(graph[vd].property);
        //if (io_evt.kind == io_event_kind::read ||
                //io_evt.kind == io_event_kind::write)
        //{
            //const std::string& file = io_evt.filename;
            //rw_per_file[file].push_back(vd);
        //}
    //}
    //return rw_per_file;
//}

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
std::map<fs::path, std::vector<VertexDescriptor>>
creates_per_dir(const IoGraph& graph, const std::vector<VertexDescriptor>& create_evts)
{
    std::map<fs::path, std::vector<VertexDescriptor>> creates_in_dir;
    for (const auto create_evt : create_evts)
    {
        auto io_evt = boost::get<io_event_property>(graph[create_evt].property);

        fs::path p(io_evt.filename);
        p.remove_filename();
        creates_in_dir[p].push_back(create_evt);
    }
    return creates_in_dir;
}

void
print_creates_in_dir(const IoGraph& graph, const std::map<fs::path, std::vector<VertexDescriptor>>& file_map, int set_count)
{
    for (auto kvp : file_map)
    {
        std::cout <<  kvp.first << " " << kvp.second.size() << "\n";
        //for (VertexDescriptor vd : kvp.second)
        //{
            //auto io_evt = get_io_property(graph, vd);
            //std::cout << io_evt << std::endl;
        //}
    }
    std::cout << "\n";
}

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trc_file);
    // get cio-sets
    auto cio_sets = find_cio_sets(graph);
    int set_cnt = 0;
    for (auto& cio_set : cio_sets)
    {
        const auto create_evts = get_io_events_by_kind(graph, cio_set, io_event_kind::create);
        auto cpd = creates_per_dir(graph, create_evts);
        print_creates_in_dir(graph, cpd, set_cnt);
        set_cnt++;
    }

    return EXIT_SUCCESS;
}

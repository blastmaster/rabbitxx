#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/utils.hpp>
#include <rabbitxx/log.hpp>

#include <string>
#include <iostream>

using namespace rabbitxx;

/**
 * open-per-file
 * Find all creates made within an cio-set in the same directory.
 *
 * handle, dir, operation_mode
 * 1. get cio-sets
 * 2. get file_map for each set
 * 3. if within same cio-set and within same directory -> store
 *
 */
std::map<fs::path, std::vector<VertexDescriptor>>
open_per_file(const IoGraph& graph, const std::vector<VertexDescriptor>& create_evts)
{
    std::map<fs::path, std::vector<VertexDescriptor>> creates_in_dir;
    for (const auto create_evt : create_evts)
    {
        auto io_evt = boost::get<io_event_property>(graph[create_evt].property);

        fs::path p(io_evt.filename);
        creates_in_dir[p].push_back(create_evt);
    }
    return creates_in_dir;
}

// This version prints the events
void
print_open_per_file(const IoGraph& graph, const std::map<fs::path, std::vector<VertexDescriptor>>& file_map, int set_count)
{
    std::cout << "==================== " << set_count << " ====================\n";
    for (auto kvp : file_map)
    {
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << "File: " << kvp.first << "\n";
        //TODO: Need an rabbitxx::io_event_stream_iterator
        //std::copy(kvp.second.begin(), kvp.second.end(),
                //std::ostream_iterator<io_event_property>(std::cout, "\n"));
        for (VertexDescriptor vd : kvp.second)
        {
            auto io_evt = get_io_property(graph, vd);
            std::cout << io_evt << std::endl;
        }
    }
}

// This version prints a count
void
print_open_per_file_count(const std::map<fs::path, std::vector<VertexDescriptor>>& file_map)
{
    for (auto kvp : file_map)
    {
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << "File: " << kvp.first << " " << kvp.second.size() << " times opend\n";
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
    auto graph = make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    // get cio-sets
    auto cio_sets = find_cio_sets(graph);
    int set_cnt = 0;
    for (auto& cio_set : cio_sets)
    {
        std::cout << "start event " << (graph)[cio_set.start_event()].timestamp() << "\n";
        const auto create_evts = get_io_events_by_kind(graph, cio_set, rabbitxx::io_event_kind::create);
        auto opf = open_per_file(graph, create_evts);
        print_open_per_file_count(opf);
        set_cnt++;
    }

    return EXIT_SUCCESS;
}

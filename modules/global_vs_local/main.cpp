#include <rabbitxx/stats.hpp>
#include <rabbitxx/utils.hpp>

using namespace rabbitxx;

/**
 * Stupid way to decide if a file resides on the global or local file system.
 * @param The file system name where the file resides on.
 * @return A std::string which contains either "local" or "global" respectively.
 */
std::string classify_fs(const std::string& str)
{
    if (str == "nfs4" || str == "autofs" || str == "lustre")
    {
        return {"global"};
    }
    return {"local"};
}

/**
 * Get the file system name where a file resides on.
 * @param IoGraph a reference to an IoGraph object.
 * @param filename a std::string containing the file name
 * @return A std::string containing the file system name of the filename.
 */
std::string get_fs_from_file(const IoGraph& graph, const std::string& filename)
{
    auto fs_map = graph.graph_properties().file_to_fs;
    return fs_map[filename];
}

std::vector<VertexDescriptor>
read_access(const IoGraph& graph,
        const std::string& filename,
        const std::vector<VertexDescriptor>& hits)
{
    std::vector<VertexDescriptor> reads;
    std::copy_if(hits.begin(), hits.end(),
            std::back_inserter(reads),
            Io_Operation_Filter(graph, io_event_kind::read));
    return reads;
}

void print_access_scopes_per_file(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    auto fm = file_map(graph, cio_set);
    for (const auto& kvp : fm)
    {
        const auto fs_str = get_fs_from_file(graph, kvp.first);
        std::cout << "File: " << kvp.first << " access on " << classify_fs(fs_str) << "\n";
    }
}

void print_read_access_per_file(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    auto fm = file_map(graph, cio_set);
    for (const auto& kvp : fm)
    {
        const auto reads = read_access(graph, kvp.first, kvp.second);
        std::cout << "File: " << kvp.first << " read ops: " << reads.size() << "\n";
        for (const auto evt : reads)
        {
            std::cout << graph[evt] << "\n";
        }
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
    auto cio_sets = find_cio_sets(graph);
    for (const auto& cio_set : cio_sets)
    {
        std::cout << "========================================\n";
        print_access_scopes_per_file(graph, cio_set);
        print_read_access_per_file(graph, cio_set);
    }

    return EXIT_SUCCESS;
}

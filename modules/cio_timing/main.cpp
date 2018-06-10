#include <chrono>
#include <rabbitxx/stats.hpp>

using namespace rabbitxx;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);

    // measure graph construction
    auto start_graph_construction = std::chrono::system_clock::now();
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trc_file);
    auto end_graph_construction = std::chrono::system_clock::now();

    // measure cio sets per process
    auto start_set_per_proc = std::chrono::system_clock::now();
    auto sets_pp = cio_sets_per_process(graph);
    auto end_set_per_proc = std::chrono::system_clock::now();

    auto start_set_merge = std::chrono::system_clock::now();
    auto cio_sets = find_cio_sets(graph, sets_pp);
    auto end_set_merge = std::chrono::system_clock::now();


    // summary
    auto graph_elapsed = end_graph_construction - start_graph_construction;
    auto sets_pp_elapsed = end_set_per_proc - start_set_per_proc;
    auto merge_elapsed = end_set_merge - start_set_merge;

    std::cout << "Time for Graph construction: " << otf2::chrono::duration_cast<otf2::chrono::microseconds>(graph_elapsed) << "\n";
    std::cout << "Time for Sets per Process: " << otf2::chrono::duration_cast<otf2::chrono::microseconds>(sets_pp_elapsed) << "\n";
    std::cout << "Time for Set merge: " << otf2::chrono::duration_cast<otf2::chrono::microseconds>(merge_elapsed) << "\n";

    return EXIT_SUCCESS;
}

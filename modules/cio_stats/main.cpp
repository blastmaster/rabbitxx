#include <rabbitxx/log.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>

#include <iostream>

using rabbitxx::logging;
using rabbitxx::VertexDescriptor;

/**
 * raw size: 53[DEBUG]: w/o empty sets: 13[DEBUG]: unique sets: 13Number of Cio-Sets: 13
 * Sum of Events in Cio-Sets: 346292
 * Set Sizes: 94770, 71092, 59239, 47374, 35533, 23686, 11847, 736, 656, 646, 10, 642, 61,
**/

void print_cio_set_stats(const rabbitxx::set_container_t<VertexDescriptor>& cio_sets)
{
    auto n_sets = cio_sets.size();
    std::vector<VertexDescriptor> set_sizes(n_sets);
    std::transform(cio_sets.begin(), cio_sets.end(), set_sizes.begin(),
            [](const auto& cio_set) {
                return cio_set.size();
            });

    std::cout << "Number of Cio-Sets: " << n_sets << std::endl;
    std::cout << "Sum of Events in Cio-Sets: "
        << std::accumulate(set_sizes.begin(), set_sizes.end(), 0)
        << std::endl;
    std::cout << "Set Sizes: ";
    std::copy(set_sizes.begin(), set_sizes.end(),
            std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: " << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    // get cio-sets
    auto cio_sets = rabbitxx::find_cio_sets(*graph);
    print_cio_set_stats(cio_sets);

    return EXIT_SUCCESS;
}

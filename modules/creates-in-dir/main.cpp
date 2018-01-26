#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
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
    static unsigned long count = 0;

    for (auto& cs : cio_sets)
    {
        for (rabbitxx::VertexDescriptor vd : cs)
        {
            try
            {
                auto property = boost::get<rabbitxx::io_event_property>((*graph)[vd].property);
                auto op = boost::get<rabbitxx::io_creation_option_container>(property.option);
                //here we have a create
                //logging::debug() << property;
                count++;
            } catch (...) {
            }
        }
    }

    std::cout << "\n\n";
    logging::debug() << "create events: " << count << "\n";

    return EXIT_SUCCESS;
}

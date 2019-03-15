#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <boost/program_options.hpp>

using rabbitxx::logging;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
    std::string trc_file;
    std::string filename;

    po::options_description description("print_graph - Print graph to dot");

    description.add_options()
        ("help,h", "Display help message")
        ("outfile,o",
         po::value<std::string>(&filename)->default_value("stdout"),
         "Output dot file")
        ("trace-file",
         po::value<std::string>(&trc_file),
         "Input trace file *.otf2");

    po::positional_options_description pd;
    pd.add("trace-file", -1);
    po::variables_map vm;

    po::store(po::command_line_parser(argc, argv).options(description).positional(pd).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cerr << description;
        return EXIT_SUCCESS;
    }

    if (!vm.count("trace-file"))
    {
        logging::error() << "No trace file given.";
        std::cerr << description;
        return EXIT_FAILURE;
    }

    auto g = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    std::cerr << "Write graph to: " << filename << "\n";

    if (filename == "stdout")
    {
        rabbitxx::write_graph_to_dot(*g.get(), std::cout);
    }
    if (filename == "stderr")
    {
        rabbitxx::write_graph_to_dot(*g.get(), std::cerr);
    }
    rabbitxx::write_graph_to_dot(*g.get(), filename);

    return 0;
}

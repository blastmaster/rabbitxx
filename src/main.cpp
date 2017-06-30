// rabbitxx
#include <rabbitxx/graph.hpp>
#include <rabbitxx/trace/graph_builder.hpp>
#include <rabbitxx/log.hpp>

// OTF2
#include <otf2xx/otf2.hpp>

// Boost
#include <boost/program_options.hpp>
#include <boost/mpi.hpp>


using rabbitxx::logging;

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    namespace po = boost::program_options;
    po::options_description description("rabbitxx");

    description.add_options()
        ("help,h", "Display help message")
        ("version", "Display version number")
        ("input-trace,i", po::value<std::string>(), "Input trace, must be an valid *.otf2 file.");

    po::variables_map vm;

    try
    {
        po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
        po::notify(vm);

        if (vm.count("help") || !vm.count("input-trace")) {
            if (world.rank() == 0) {
                std::cout << description;
            }
            return 0;
        }

        const auto input_trace = vm["input-trace"].as<std::string>();
        otf2::reader::reader reader(input_trace);

        std::size_t num_locations = reader.num_locations();
        logging::debug() << "OTF2 trace reader created... " << num_locations << " locations.";

        //rabbitxx::log::set_min_severity_level("trace");

        //TODO build communicator to read the trace into the graph!?!?

        //TODO build graphbuilder
        rabbitxx::trace::graph_builder<rabbitxx::Graph> graph_builder(world);
        //TODO set callbacks to graphbuilder
        reader.set_callback(graph_builder); //FIXME: when passing true as second parameter something went wrong

        reader.read_definitions();

        //TODO barrier here!
        world.barrier(); // wait until each process has read the definition records
        //reader.read_events();

        logging::info() << "Trace processing done.";

    }
    catch (std::exception& e)
    {
        logging::fatal() << "Exception raised: " << e.what();
        env.abort(1);
        return 1;
    }
    catch (...) // catch-all case
    {
        logging::fatal() << "Unknown exception raised. Abort... X(";
        env.abort(2);
        return 2;
    }

    return 0;
}

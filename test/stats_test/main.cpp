#include <rabbitxx/stats.hpp>
#include <rabbitxx/experiment.hpp>

using namespace rabbitxx;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: " << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    fs::path trc_file(argv[1]);
    Experiment exp(trc_file); // just trace file use default config
    auto stats = exp.run();


    return EXIT_SUCCESS;
}

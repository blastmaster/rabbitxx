#ifndef RABBITXX_EXPERIMENT_HPP
#define RABBITXX_EXPERIMENT_HPP

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_types.hpp>
#include <rabbitxx/stats.hpp>
#include <rabbitxx/utils.hpp>
#include <functional>

/**
 *
 * /experiment-directory
    /info
        /summary.csv
    /cio-sets
        /cio-set0.csv
        /...
        /cio-setN.csv
  /pio-sets
    /0
        /pio-set0.csv
        /...
        /pio-setN.csv
    /...
    /N
 **/

namespace rabbitxx
{

struct experiment_config
{
    bool with_summary = true;
    bool pio_sets = false;
    bool cio_sets = true;
};

inline std::ostream& operator<<(std::ostream& os, const experiment_config& ec)
{
    os << std::boolalpha << "with_summary: " << ec.with_summary
        << "pio_set: " << ec.pio_sets
        << "cio_set: " << ec.cio_sets;
    return os;
}

class Experiment
{
public:
    struct experiment_results
    {
        IoGraph graph;
        set_container_t<VertexDescriptor> cio_sets;
        set_map_t<VertexDescriptor> pio_sets;
    };

    using output_func = std::function<
        void(const Experiment::experiment_results&,
            const Experiment_Stats&,
            const experiment_config&,
            const fs::path&)>;


    static experiment_config make_default_config()
    {
        return experiment_config();
    }

    static std::string make_default_experiment_name()
    {
        auto time = std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now());
        std::stringstream exp_name;
        exp_name << "rabbitxx-" << std::put_time(
                std::localtime(&time), "%F-%H-%M-%S");
        return exp_name.str();
    }

    explicit Experiment(const fs::path& input_trace) : trace_file_(input_trace),
        experiment_name_(make_default_experiment_name()), experiment_dir_(make_default_path()),
        config_(Experiment::make_default_config())
    {
    }

    Experiment(const fs::path& input_trace, const fs::path& out_dir) : trace_file_(input_trace),
        experiment_name_(make_default_experiment_name()), experiment_dir_(make_path(out_dir)),
        config_(Experiment::make_default_config())
    {
    }

    Experiment(const fs::path& input_trace, const fs::path& out_dir, const std::string& name, const experiment_config& conf) : trace_file_(input_trace),
        experiment_name_(name), experiment_dir_(make_path(out_dir)),
        config_(conf)
    {
    }

    Experiment_Stats run();

    Experiment_Stats run(output_func out_f);

private:

    fs::path make_default_path() const;

    fs::path make_path(const fs::path& p) const;

    Experiment_Stats run_experiment(experiment_results& results);

private:
    fs::path trace_file_;
    std::string experiment_name_;
    fs::path experiment_dir_;
    experiment_config config_;
};

void create_directory_structure(const fs::path& base_path, const experiment_config& config);

// default output function for Experiment, using ostream& << operator
void experiment_output(const Experiment::experiment_results& res,
        const Experiment_Stats& stats,
        const experiment_config& config,
        const fs::path& base_path);

} // namespace rabbitxx

#endif // RABBITXX_EXPERIMENT_HPP

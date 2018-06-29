#include <rabbitxx/experiment.hpp>

namespace rabbitxx
{

void create_directory_structure(const fs::path& base_path, const experiment_config& config)
{
    if (!fs::exists(base_path))
    {
        // TODO: use system::error_code overload for proper error-handling
        fs::create_directory(base_path);
    }
    if (config.cio_sets)
    {
        fs::create_directory(base_path / "cio-sets");
    }
    if (config.pio_sets)
    {
        fs::create_directory(base_path / "pio-sets");
    }
}

Experiment_Stats Experiment::run(output_func out_f=experiment_output)
{
    // create dir structure
    create_directory_structure(experiment_dir_, config_);
    const auto experiment_start_ts = std::chrono::system_clock::now();
    experiment_results res;
    auto experiment_stats = run_experiment(res);
    const auto experiment_end_ts = std::chrono::system_clock::now();
    auto experiment_dur = experiment_end_ts - experiment_start_ts;
    experiment_stats.set_experiment_duration(experiment_dur);
    out_f(res, experiment_stats, config_, experiment_dir_);
    return experiment_stats;
}

Experiment_Stats Experiment::run()
{
    return this->run(experiment_output);
}

fs::path Experiment::make_default_path() const
{
    return fs::current_path() / experiment_name_;
}

fs::path Experiment::make_path(const fs::path& p) const
{
    return p / experiment_name_;
}

Experiment_Stats Experiment::run_experiment(experiment_results& results)
{
    // graph construction
    const auto start_graph_construction = std::chrono::system_clock::now();
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trace_file_.string());
    const auto end_graph_construction = std::chrono::system_clock::now();
    const auto graph_duration = end_graph_construction - start_graph_construction;
    auto graph_stats = Graph_Stats(graph, graph_duration);
    auto info = graph.graph_properties();

    if (config_.pio_sets || config_.cio_sets)
    {
        // measure cio sets per process
        const auto start_set_per_proc = std::chrono::system_clock::now();
        auto sets_pp = cio_sets_per_process(graph);
        const auto end_set_per_proc = std::chrono::system_clock::now();
        const auto duration = end_set_per_proc - start_set_per_proc;
        auto pio_stats = PIO_Stats(graph, sets_pp, duration);

        const auto start_set_merge = std::chrono::system_clock::now();
        auto cio_sets = find_cio_sets(graph, sets_pp);
        const auto end_set_merge = std::chrono::system_clock::now();
        const auto cio_duration = end_set_merge - start_set_merge;
        auto cio_stats = CIO_Stats(graph, cio_sets, cio_duration);

        results.graph = std::move(graph);
        results.cio_sets = std::move(cio_sets);
        results.pio_sets = std::move(sets_pp);
        return Experiment_Stats(trace_file_, graph_stats, cio_stats, pio_stats, info.file_to_fs, info.clock_props);
    }

}

// default output function for Experiment, using ostream& << operator
void experiment_output(const Experiment::experiment_results& res,
        const Experiment_Stats& stats,
        const experiment_config& config,
        const fs::path& base_path)
{
    if (config.pio_sets)
    {
        std::cout << "PIO-Sets\n";
        logging::debug() << "Default output not implemented yet!";
        // output pio_sets
    }
    if (config.cio_sets)
    {
        // output cio_sets
        std::cout << "CIO-Sets\n";
        std::copy(std::begin(res.cio_sets), std::end(res.cio_sets),
                std::ostream_iterator<set_t<VertexDescriptor>>(std::cout, "\n"));
        std::cout << "\n";
    }
    if (config.with_summary)
    {
        std::cout << "Summary:\n";
        std::cout << stats;
    }
}

} // namespace rabbitxx

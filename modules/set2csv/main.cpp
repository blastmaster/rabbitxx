#include <rabbitxx/experiment.hpp>

/**
 * Module to serialize sets to csv
 * This module prints all io events within a set into csv.
 *
 * TODO
 * * options have different length and are therefore omitted at the moment 
 * * header() does not work, one comma to much at the end.
 */

using namespace rabbitxx;

//TODO unused, one comma to much.
void header(std::ostream& os=std::cout)
{
    std::array<const char*, 10> columns {
        "pid",
        "filename",
        "region_name",
        "paradigm",
        "request_size",
        "response_size",
        "offset",
        //"option",
        "kind",
        "timestamp"};
    std::copy(columns.begin(), columns.end(), std::ostream_iterator<const char*>(os, ", "));
    os << "\n";
}

struct option_csv_printer : boost::static_visitor<std::string>
{
    std::string operator()(const io_operation_option_container& option) const
    {
        std::stringstream sstr;
        sstr << to_string(option.op_mode) << ", "
            << to_string(option.op_flag);

        return sstr.str();
    }

    std::string operator()(const io_creation_option_container& option) const
    {
        std::stringstream sstr;
        sstr << to_string(option.status_flag) << ", "
            << to_string(option.creation_flag) << ", ";
        if (option.access_mode)
        {
            sstr << to_string(option.access_mode.value()) << ", ";
        }

        return sstr.str();
    }

    std::string operator()(const otf2::common::io_seek_option_type& option) const
    {
        return to_string(option);
    }
};

std::ostream& io_event_2_csv_stream(const IoGraph& graph, const VertexDescriptor& evt, std::ostream& out)
{
    const auto& io_evt = get_io_property(graph, evt);
    out << io_evt.proc_id << ", "
        << io_evt.filename << ", "
        << io_evt.region_name << ", "
        << io_evt.paradigm << ", "
        << io_evt.request_size << ", "
        << io_evt.response_size << ", "
        << io_evt.offset << ", "
        //<< boost::apply_visitor(option_csv_printer(), evt.option) << ", "
        << io_evt.kind << ", "
        //<< graph[evt].duration.duration << ", "
        << otf2::chrono::duration_cast<otf2::chrono::nanoseconds>(graph[evt].duration.duration).count() << ", "
        << io_evt.timestamp;

    return out;
}

void set2csv(const IoGraph& graph, const set_t<VertexDescriptor>& set, std::ostream& out=std::cout)
{
    for (auto evt : set)
    {
        io_event_2_csv_stream(graph, evt, out);
        out << "\n";
    }

}

static std::string create_cio_set_stats_filename(std::uint64_t sidx)
{
    std::stringstream sstr;
    sstr << "set-" << sidx << ".csv";

    return sstr.str();
}

void sets_2_csv(const IoGraph& graph, const set_container_t<VertexDescriptor>& cio_sets,
         const fs::path& base_path)
{
    //header();
    std::uint64_t idx = 1;
    for (const auto& set : cio_sets)
    {
        std::string filename = create_cio_set_stats_filename(idx);
        std::ofstream ofile((base_path / filename).string());
        set2csv(graph, set, ofile);
        ++idx;
    }
}

void pio_sets_2_csv(const IoGraph& graph, const set_map_t<VertexDescriptor>& pio_sets,
        const fs::path& base_path)
{
    for (const auto p_kvp : pio_sets)
    {
        fs::path proc_dir = base_path / std::to_string(p_kvp.first);
        fs::create_directory(proc_dir);
        sets_2_csv(graph, p_kvp.second, proc_dir);
    }
}

void cio_stats_2_csv(const CIO_Stats& stats, std::ostream& out)
{
    out << "Number of CIO-Sets," << stats.number_of_cio_sets() << "\n"
        << "Build time," << stats.build_time() << "\n"
        << "Set Durations,";
    const auto set_durs = stats.get_set_durations();
    // one comma to much
    std::copy(set_durs.begin(), set_durs.end(),
            std::ostream_iterator<otf2::chrono::duration>(out, ","));
    out << "\n";
}

void pio_stats_2_csv(const PIO_Stats& stats, std::ostream& out)
{
    out << "Number of PIO-Sets," << stats.number_of_pio_sets() << "\n"
        << "PIO-Sets build time," << stats.build_time() << "\n";
}

void graph_stats_2_csv(const Graph_Stats& stats, std::ostream& out)
{
    out << "Number of Vertices," << stats.number_of_vertices() << "\n"
        << "Number of Edges," << stats.number_of_edges() << "\n"
        << "Graph Build Time," << stats.build_time() << "\n"
        << "Ticks per Second," << stats.ticks_per_second().count() << "\n"
        << "Start Time," << stats.start_time().count() << "\n"
        << "Length," << stats.length().count() << "\n";
}

void experiment_stats_2_csv(const Experiment_Stats& stats, std::ostream& out)
{
    out << "Tracefile," << stats.trace_file() << "\n"
        << "Experiment Duration," << stats.experiment_duration() << "\n";
    graph_stats_2_csv(stats.graph_stats(), out);
    cio_stats_2_csv(stats.cio_stats(), out);
    pio_stats_2_csv(stats.pio_stats(), out);
}

void summary_2_csv(const Experiment_Stats& stats, const fs::path& base_path)
{
    std::ofstream out_file((base_path / "summary.csv").string());
    experiment_stats_2_csv(stats, out_file);
}

void csv_output(const Experiment::experiment_results& res,
        const Experiment_Stats& stats,
        const experiment_config& config,
        const fs::path& base_path)
{
    if (config.pio_sets)
    {
        pio_sets_2_csv(res.graph, res.pio_sets, base_path / "pio-sets");
    }
    if (config.cio_sets)
    {
        sets_2_csv(res.graph, res.cio_sets, base_path /  "cio-sets");
    }
    if (config.with_summary)
    {
        summary_2_csv(stats, base_path);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    fs::path trc_file(argv[1]);
    Experiment exp(trc_file); // just trace file use default config
    auto stats = exp.run(csv_output);
    return EXIT_SUCCESS;
}

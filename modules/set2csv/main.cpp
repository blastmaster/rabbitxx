#include <rabbitxx/experiment.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

#include <boost/program_options.hpp>

/**
 * Module to serialize sets to csv
 * This module prints all io events within a set into csv.
 *
 * TODO
 * * options have different length and are therefore omitted at the moment 
 * * header() does not work, one comma to much at the end.
 */

using namespace rabbitxx;
namespace po = boost::program_options;

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
        << std::chrono::duration_cast<otf2::chrono::microseconds>(graph[evt].duration.duration).count() << ", "
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
        << "CIO-Sets build time," << std::chrono::duration_cast<otf2::chrono::microseconds>(stats.build_time()) << "\n"
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
        << "PIO-Sets build time," << std::chrono::duration_cast<otf2::chrono::microseconds>(stats.build_time()) << "\n";
}

void graph_stats_2_csv(const Graph_Stats& stats, std::ostream& out)
{
    out << "Number of Vertices," << stats.number_of_vertices() << "\n"
        << "Number of Edges," << stats.number_of_edges() << "\n"
        << "Graph Build Time," << std::chrono::duration_cast<otf2::chrono::microseconds>(stats.build_time()) << "\n"
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

template<typename JsonWriter>
void graph_stats_to_json(const Graph_Stats& stats, JsonWriter& writer)
{
    writer.StartObject();
    writer.Key("Number of Vertices");
    writer.Uint64(stats.number_of_vertices());
    writer.Key("Number of Edges");
    writer.Uint64(stats.number_of_edges());
    writer.Key("Build time");
    writer.Uint64(
            std::chrono::duration_cast<otf2::chrono::microseconds>(
                stats.build_time())
            .count());
    writer.Key("Ticks per Seconds");
    writer.Uint64(stats.ticks_per_second().count());
    writer.Key("Start Time");
    writer.Uint64(stats.start_time().count());
    writer.Key("Length");
    writer.Uint64(stats.length().count());
    writer.EndObject();
}

template<typename JsonWriter>
void cio_stats_to_json(const CIO_Stats& stats, JsonWriter& writer)
{
    writer.StartObject();
    writer.Key("Number of CIO-Sets");
    writer.Uint64(stats.number_of_cio_sets());
    writer.Key("Build time");
    writer.Uint64(
            std::chrono::duration_cast<otf2::chrono::microseconds>(
                stats.build_time())
            .count());
    writer.Key("Set durations");
    writer.StartArray();
    const auto set_durations = stats.get_set_durations();
    for (const auto& set_duration : set_durations)
    {
        writer.Uint64(
                std::chrono::duration_cast<otf2::chrono::microseconds>(set_duration)
                .count());
    }
    writer.EndArray();
    writer.EndObject();
}

template<typename JsonWriter>
void pio_stats_to_json(const PIO_Stats& stats, JsonWriter& writer)
{
    writer.StartObject();
    writer.Key("Number of PIO-Sets");
    writer.Uint64(stats.number_of_pio_sets());
    writer.Key("Build time");
    writer.Uint64(
            std::chrono::duration_cast<otf2::chrono::microseconds>(
                stats.build_time())
            .count());
    writer.EndObject();
}

void experiment_stats_to_json(const Experiment_Stats& stats, std::ostream& out)
{
    rapidjson::OStreamWrapper osw(out);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    writer.StartObject();
    writer.Key("Tracefile");
    writer.String(stats.trace_file().string().c_str());
    writer.Key("Experiment_Duration");
    writer.Uint64(stats.experiment_duration().count());

    writer.Key("Graph Stats");
    graph_stats_to_json(stats.graph_stats(), writer);
    writer.Key("CIO Stats");
    cio_stats_to_json(stats.cio_stats(), writer);
    writer.Key("PIO Stats");
    pio_stats_to_json(stats.pio_stats(), writer);
    writer.EndObject();
}

void summary_to_json(const Experiment_Stats& stats, const fs::path& base_path)
{
    std::ofstream out_file((base_path / "summary.json").string());
    experiment_stats_to_json(stats, out_file);
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
        summary_to_json(stats, base_path);
    }
}

int main(int argc, char** argv)
{
    fs::path base_path;
    fs::path trc_file;
    std::string experiment_name;
    bool with_summary;
    bool cio_set_out, pio_set_out;

    po::options_description description("set2csv - Output CIO-Sets as csv");

    // clang-format off
    description.add_options()
        ("help,h", "Display help message")
        ("summary,s",
            po::bool_switch(&with_summary)->default_value(true),
            "Output summary")
        ("out-dir,o",
            po::value<fs::path>(&base_path)->default_value(fs::current_path()),
            "Output base path")
        ("name,n",
            po::value<std::string>(&experiment_name)
            ->default_value(Experiment::make_default_experiment_name()),
            "Experiment name")
        ("cio-sets,c",
            po::bool_switch(&cio_set_out)->default_value(true),
            "Output CIO-Sets, global Concurrent I/O Sets")
        ("pio-sets,p",
            po::bool_switch(&pio_set_out)->default_value(false),
            "Output PIO-Sets, local sets per-process")
        ("trace-file",
            po::value<fs::path>(&trc_file),
            "Input trace file *.otf2");
    // clang-formant on

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

    if (with_summary)
    {
        // enable or disable summary
        logging::debug() << "enable summary";
    }

    if (cio_set_out)
    {
        // enable or disable cio-set output
        logging::debug() << "enable cio-set output";
    }

    if (pio_set_out)
    {
        // enable or disable pio-set output
        logging::debug() << "enable pio-set output";
    }

    experiment_config e_conf {with_summary, pio_set_out, cio_set_out};
    logging::debug() << "created config: " << e_conf;

    if (vm.count("out-dir"))
    {
        // set base path to out-dir
        logging::debug() << "using outdir: " << base_path.string();
    }

    if (!vm.count("trace-file"))
    {
        logging::error() << "need trace file!";
        return EXIT_FAILURE;
    }

    if (vm.count("trace-file"))
    {
        // tracefile given, otherwise this is an error!
        logging::debug() << "using tracefile: " << trc_file.string();
    }

    //Experiment exp(trc_file); // just trace file use default config
    Experiment exp(trc_file, base_path, experiment_name, e_conf);
    auto stats = exp.run(csv_output);
    return EXIT_SUCCESS;
}

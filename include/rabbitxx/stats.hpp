#ifndef RABBITXX_STATS_HPP
#define RABBITXX_STATS_HPP

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/utils.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

namespace rabbitxx
{


class [[deprecated]] File
{
public:
    File(std::string filename, std::string file_system) : filename_(filename), file_system_(file_system)
    {}

    File(std::string filename, const IoGraph& graph) : filename_(filename)
    {
        auto fs_map = graph.graph_properties().file_to_fs;
        file_system_ = fs_map[filename];
    }

    const std::string filename() const
    {
        return filename_;
    }

    const std::string file_system() const
    {
        return file_system_;
    }

private:
    std::string filename_;
    std::string file_system_;
};

/**
 * Function object to filter for certain `io_event_kind` types.
 */

struct [[deprecated]] Io_Operation_Filter
{
    Io_Operation_Filter(const IoGraph& graph, const io_event_kind filter_kind) : kind_(filter_kind), graph_(graph) {}
    bool operator() (const VertexDescriptor& vd) const
    {
        auto io_prop = get_io_property(graph_, vd);
        return io_prop.kind == kind_;
    }

    io_event_kind kind_;
    const IoGraph& graph_;
};


std::map<std::string, std::vector<VertexDescriptor>>
file_map(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set);


/**
 * @brief Get the duration of a cio-set in microseconds.
 * The duration is the difference between the end_event timestamp and the start_event timestamp.
 *
 * @param graph an IoGraph const-ref
 * @param cio_set the cio set container
 * @return the duration of the cio-set container in `otf2::chrono::microseconds`.
 */
// TODO: timestamp of the first and the last I/O event within the set.
otf2::chrono::microseconds
get_set_duration(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set);


/**
 * @class Type gathering general statistics of `CIO_Sets`.
 *
 **/
class CIO_Stats
{
public:

    CIO_Stats(const IoGraph& graph, const set_container_t<VertexDescriptor>& cio_sets,
            const otf2::chrono::duration& build_tm) :
        num_cio_sets_(cio_sets.size()), build_time_(build_tm)
    {
        std::transform(cio_sets.begin(), cio_sets.end(), std::back_inserter(set_durations_),
                [&graph](const auto& set) { return get_set_duration(graph, set); });
    }

    std::uint64_t number_of_cio_sets() const
    {
        return num_cio_sets_;
    }

    otf2::chrono::duration get_nth_set_duration(std::size_t n) const
    {
        return set_durations_[n];
    }

    std::vector<otf2::chrono::duration> get_set_durations() const
    {
        return set_durations_;
    }

    otf2::chrono::duration build_time() const
    {
        return build_time_;
    }

private:
    std::uint64_t num_cio_sets_;
    otf2::chrono::duration build_time_;
    std::vector<otf2::chrono::duration> set_durations_;
};

inline std::ostream& operator<<(std::ostream& os, const CIO_Stats& stats)
{
    os << "========== CIO-Set Stats ==========\n"
        << "Number of CIO-Sets: " << stats.number_of_cio_sets() << "\n"
        << "CIO-Stats build time: " << std::chrono::duration_cast<otf2::chrono::microseconds>(stats.build_time()) << "\n";
    const auto set_durs = stats.get_set_durations();
    std::copy(set_durs.begin(), set_durs.end(), std::ostream_iterator<otf2::chrono::duration>(os, ", "));
    os << "\n";

    return os;
}

/**
 * @class Type gathering general statistics of `PIO_Sets`
 *
 **/
class PIO_Stats
{
public:
    PIO_Stats(const IoGraph& graph, const set_map_t<VertexDescriptor>& pio_sets,
            const otf2::chrono::duration& build_tm) :
        num_pio_sets_(pio_sets.size()), build_time_(build_tm)
    {
    }

    std::uint64_t number_of_pio_sets() const
    {
        return num_pio_sets_;
    }

    otf2::chrono::duration build_time() const
    {
        return build_time_;
    }

private:
    //std::vector<otf2::chrono::duration> set_durations_;
    std::uint64_t num_pio_sets_;
    otf2::chrono::duration build_time_;
};

inline std::ostream& operator<<(std::ostream& os, const PIO_Stats& stats)
{
    os <<  "========== PIO-Sets Stats ==========\n"
        << "Number of PIO-Sets: " << stats.number_of_pio_sets() << "\n"
        << "PIO-Sets Build time: " << std::chrono::duration_cast<otf2::chrono::microseconds>(stats.build_time()) << "\n";

    return os;
}

class Graph_Stats
{
public:
    explicit Graph_Stats(const IoGraph& graph, const otf2::chrono::duration& build_tm) : 
        num_vertices_(graph.num_vertices()), num_edges_(graph.num_edges()),
        build_time_(build_tm)
    {
    }

    std::uint64_t number_of_vertices() const
    {
        return num_vertices_;
    }

    std::uint64_t number_of_edges() const
    {
        return num_edges_;
    }

    otf2::chrono::duration build_time() const
    {
        return build_time_;
    }

private:
    std::uint64_t num_vertices_;
    std::uint64_t num_edges_;
    otf2::chrono::duration build_time_;
};

inline std::ostream& operator<<(std::ostream& os, const Graph_Stats& stats)
{
    os << "========== Graph Stats ==========\n"
        << "Number of Vertices: " << stats.number_of_vertices() << "\n"
        << "Number of Edges: " << stats.number_of_edges() << "\n"
        << "Graph Build time: " << std::chrono::duration_cast<otf2::chrono::microseconds>(stats.build_time()) << "\n";

    return os;
}

class Experiment_Stats
{
    using fs_map_t = std::map<std::string, std::string>;
public:
    Experiment_Stats(const fs::path& input_trace,
                    const Graph_Stats& graph_stats,
                    const CIO_Stats& cio_stats,
                    const PIO_Stats& pio_stats,
                    const fs_map_t& file_map,
                    const otf2::definition::clock_properties& clock_properties,
                    std::uint64_t num_locations)
        : trace_file_(input_trace),
        graph_stats_(graph_stats),
        cio_stats_(cio_stats),
        pio_stats_(pio_stats),
        file_map_(file_map),
        clock_props_(clock_properties),
        num_locations_(num_locations)
    {
    }

    fs::path trace_file() const
    {
        return trace_file_;
    }

    Graph_Stats graph_stats() const
    {
        return graph_stats_;
    }

    CIO_Stats cio_stats() const
    {
        return cio_stats_;
    }

    PIO_Stats pio_stats() const
    {
        return pio_stats_;
    }

    fs_map_t file_system_map() const
    {
        return file_map_;
    }

    std::uint64_t num_locations() const
    {
        return num_locations_;
    }

    otf2::definition::clock_properties clock_properties() const
    {
        return clock_props_;
    }

    otf2::chrono::duration experiment_duration() const
    {
        return experiment_duration_;
    }

    void set_experiment_duration(const otf2::chrono::duration& experiment_dur)
    {
        experiment_duration_ = experiment_dur;
    }

private:
    fs::path trace_file_;
    Graph_Stats graph_stats_;
    CIO_Stats cio_stats_;
    PIO_Stats pio_stats_;
    fs_map_t file_map_;
    otf2::definition::clock_properties clock_props_;
    otf2::chrono::duration experiment_duration_ = otf2::chrono::duration(0);
    std::uint64_t num_locations_ = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Experiment_Stats& stats)
{
    os << "========== Experiment Stats ==========\n"
        << "Input trace file: " << stats.trace_file() << "\n"
        << "Experiment Duration: " << stats.experiment_duration() << "\n"
        << "Number of locations: " << stats.num_locations() << "\n"
        << stats.graph_stats() << stats.pio_stats() <<  stats.cio_stats();
    //TODO: print clock properties and file system map
    return os;
}

} // namespace rabbitxx

#endif // RABBITXX_STATS_HPP

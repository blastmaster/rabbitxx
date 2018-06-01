#ifndef RABBITXX_CIO_STATS_HPP
#define RABBITXX_CIO_STATS_HPP

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

namespace rabbitxx
{

class File
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
struct Io_Operation_Filter
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

/**
 * Take a file and track accesses within a cio-set on that file.
 */
class FileAccessTracker
{
public:
    explicit FileAccessTracker(std::string filename) : filename_(filename)
    {}

    std::size_t operator()(const IoGraph& graph, const set_t<VertexDescriptor> cio_set)
    {
        for (const auto& evt : cio_set)
        {
            auto io_evt = get_io_property(graph, evt);
            if (io_evt.filename == filename_)
            {
                accessing_events_.push_back(evt);
            }
        }

        return accessing_events_.size();
    }

    // filter overload
    std::size_t operator()(const IoGraph& graph, const set_t<VertexDescriptor> cio_set,
            const Io_Operation_Filter& filter)
    {
        std::copy_if(cio_set.begin(), cio_set.end(),
                std::back_inserter(accessing_events_),
                filter);

        return accessing_events_.size();
    }

    std::string filename()
    {
        return filename_;
    }

    std::vector<VertexDescriptor> accessing_events()
    {
        return accessing_events_;
    }

private:
    std::string filename_;
    std::vector<VertexDescriptor> accessing_events_ {};
};


std::map<std::string, std::vector<VertexDescriptor>>
region_map(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<VertexDescriptor>> rm;
    for (auto vd : cio_set)
    {
        const auto io_evt = get_io_property(graph, vd);
        rm[io_evt.region_name].push_back(vd);
    }

    return rm;
}

/**
 * Build a map of keys of `io_event_kind` and a vector of vertex descriptors from a given set
 * belong the the `io_event_kind`.
 *
 * @param IoGraph a const reference to an IoGraph object.
 * @param cio_set a const reference to a CIO-Set.
 * @return A std::map with the `io_event_kind` as key, and a vector of
 * all operations belong to this kind as a value.
 */
std::map<io_event_kind, std::vector<VertexDescriptor>>
kind_map(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<io_event_kind, std::vector<VertexDescriptor>> io_evt_kind_map;
    for (const auto& evt_vd : cio_set)
    {
        auto io_evt = boost::get<io_event_property>(graph[evt_vd].property);
        switch(io_evt.kind)
        {
            case io_event_kind::create:
                io_evt_kind_map[io_event_kind::create].push_back(evt_vd);
                break;
            case io_event_kind::dup:
                io_evt_kind_map[io_event_kind::dup].push_back(evt_vd);
                break;
            case io_event_kind::read:
                io_evt_kind_map[io_event_kind::read].push_back(evt_vd);
                break;
            case io_event_kind::write:
                io_evt_kind_map[io_event_kind::write].push_back(evt_vd);
                break;
            case io_event_kind::seek:
                io_evt_kind_map[io_event_kind::seek].push_back(evt_vd);
                break;
            case io_event_kind::flush:
                io_evt_kind_map[io_event_kind::flush].push_back(evt_vd);
                break;
            case io_event_kind::delete_or_close:
                io_evt_kind_map[io_event_kind::delete_or_close].push_back(evt_vd);
                break;
            default:
                logging::debug() << "Ambigious I/O-Event in CIO_Set\n" << io_evt;
        }
    }

    return io_evt_kind_map;
}

/**
 * Print how many operation of a `io_event_kind` happen in a cio-set.
 * TODO just printing ... put me somewhere else!
 */
[[deprecated]]
void event_kinds_per_set(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    const auto km = kind_map(graph, cio_set);
    for (const auto& kvp : km)
    {
        std::cout << kvp.first << ": " << kvp.second.size() << "\n";
    }
}

std::map<std::string, std::vector<VertexDescriptor>>
file_map(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<VertexDescriptor>> f_map;
    for (VertexDescriptor vd : cio_set)
    {
        auto io_evt = get_io_property(graph, vd);
        f_map[io_evt.filename].push_back(vd);
    }
    return f_map;
}

/**
 * Get the set of processes accessing a given file within the same cio-set.
 * TODO: take a vector of vds and a file.
 * Then we don't have to call `file_map` every time, which is useless since we are just interesseted in one single file.
 */
[[deprecated]]
std::set<std::uint64_t> concurrent_accessing_processes(const IoGraph& graph,
        const set_t<VertexDescriptor>& set,
        const std::string& filename)
{
    auto fm = file_map(graph, set);
    std::set<std::uint64_t> procs;
    for (const auto evt : fm[filename])
    {
        auto pid = graph[evt].id();
        procs.insert(pid);
    }

    return procs;
}

/**
 * check wether the given file is accessed by more than one process within this set.
 */
[[deprecated]]
bool file_has_shared_access(const IoGraph& graph,
        const set_t<VertexDescriptor>& set,
        const std::string& filename)
{
    const auto procs = concurrent_accessing_processes(graph, set, filename);
    return (procs.size() > 1);
}

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
get_set_duration(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    auto start_vd = cio_set.start_event();
    auto end_vd = cio_set.end_event().value();
    auto ts_start = graph[start_vd].timestamp();
    auto ts_end = graph[end_vd].timestamp();
    if (graph[start_vd].type == vertex_kind::synthetic)
    {
        //take timestamp of first event in set
        //std::cerr << "set start event ts from: " << ts_start;
        ts_start = graph[*cio_set.begin()].timestamp();
        //std::cerr << " to " << ts_start << "\n";
    }
    if (graph[end_vd].type == vertex_kind::synthetic)
    {
        //std::cerr << "set end event ts from: " << ts_end;
        ts_end = graph[*std::prev(cio_set.end())].timestamp();
        //std::cerr << " to " << ts_end << "\n";
    }
    assert(ts_end > ts_start);
    auto dur_diff = ts_end - ts_start;
    return otf2::chrono::duration_cast<otf2::chrono::microseconds>(dur_diff);
}

/**
 * @class CIO_Stats gathers general statistics about a given set.
 */
class CIO_Stats
{
    struct rw_stats
    {
        std::uint64_t total {0};
        std::uint64_t max_size {0};
        std::uint64_t min_size {0};
        std::uint64_t sum_size {0};
        double avg_size {0.0};
        std::vector<std::uint64_t> response_sizes;

        rw_stats() = default;

        explicit rw_stats(std::uint64_t num_evts, std::uint64_t max_sz, std::uint64_t min_sz,
                std::uint64_t sum, double avg, std::vector<std::uint64_t> resp_sizes) noexcept
            : total(num_evts), max_size(max_sz), min_size(min_sz), sum_size(sum),
            avg_size(avg), response_sizes(std::move(resp_sizes))
        {
        }
    };

    otf2::chrono::microseconds duration;
    rw_stats write_stats;
    rw_stats read_stats;
    // read / write stats per read / write region
    std::map<std::string, rw_stats> region_stats;

    // NOTE: Take the response sizes, since that is whats actually written or read.
    rw_stats make_rw_stats(const IoGraph& graph,
            const std::vector<VertexDescriptor>& events)
    {
        std::uint64_t max_sz {0};
        std::uint64_t min_sz {0};
        double avg_sz {0.0};
        std::vector<std::uint64_t> response_sizes;
        std::transform(events.begin(), events.end(), std::back_inserter(response_sizes),
                [&graph](const VertexDescriptor vd)
                {
                    auto io_evt = get_io_property(graph, vd);
                    return io_evt.response_size;
                });
        auto min_sz_it = std::min_element(response_sizes.begin(), response_sizes.end());
        if (min_sz_it != events.end())
        {
            min_sz = *min_sz_it;
        }

        auto max_sz_it = std::max_element(response_sizes.begin(), response_sizes.end());
        if (max_sz_it != events.end())
        {
            max_sz = *max_sz_it;
        }
        std::uint64_t sum_sz = std::accumulate(response_sizes.begin(), response_sizes.end(), 0);
        std::uint64_t size = static_cast<std::uint64_t>(events.size());
        if (size > 0) {
            avg_sz = static_cast<double>(sum_sz / size);
        }
        return rw_stats(size, max_sz, min_sz, sum_sz, avg_sz, response_sizes);
    }


public:

    explicit CIO_Stats(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
    {
        auto k_map = kind_map(graph, cio_set);

        duration = get_set_duration(graph, cio_set);
        write_stats = make_rw_stats(graph, k_map[io_event_kind::write]);
        read_stats = make_rw_stats(graph, k_map[io_event_kind::read]);
    }


    const rw_stats& get_total_read_stats() const noexcept
    {
        return read_stats;
    }

    const rw_stats& get_total_write_stats() const noexcept
    {
        return write_stats;
    }

    const std::map<std::string, rw_stats>&
    rw_region_stats() const noexcept
    {
        return region_stats;
    }

    otf2::chrono::microseconds
    set_duration() const noexcept
    {
        return duration;
    }
};

void dump_region_stats(const CIO_Stats& stats)
{
    //TODO split region
    std::cout << "====================" << " Read / Write - Stats per region " << "====================" << "\n";
    const auto& region_stats_map = stats.rw_region_stats();
    for (const auto& region_st : region_stats_map)
    {
        std::cout << region_st.first << " " <<
            " min: " << region_st.second.min_size <<
            " max: " << region_st.second.max_size <<
            " sum: " << region_st.second.sum_size <<
            " avg: " << region_st.second.avg_size << "\n";
    }
    std::cout << "\n";
}


void dump_stats(const CIO_Stats& stats)
{
    const auto& read_stats = stats.get_total_read_stats();
    const auto& write_stats = stats.get_total_write_stats();
    std::cout << "Read-Stats: " << "\n" <<
        " Number of read events: " << read_stats.total <<
        " min: " << read_stats.min_size <<
        " max: " << read_stats.max_size <<
        " sum: " << read_stats.sum_size <<
        " avg: " << read_stats.avg_size << "\n";

    std::cout << "Write-Stats: " << "\n" <<
        " Number of write events: " << write_stats.total <<
        " min: " << write_stats.min_size <<
        " max: " << write_stats.max_size <<
        " sum: " << write_stats.sum_size <<
        " avg: " << write_stats.avg_size << "\n";

    std::cout << "\n";
}

} // namespace rabbitxx

#endif // RABBITXX_CIO_STATS_HPP

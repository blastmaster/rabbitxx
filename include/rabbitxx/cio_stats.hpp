#ifndef RABBITXX_CIO_STATS_HPP
#define RABBITXX_CIO_STATS_HPP

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

using rabbitxx::logging;

namespace rabbitxx
{

double avg_request_size(const IoGraph& graph, const std::vector<VertexDescriptor>& events)
{
    if (events.empty())
    {
        return 0.0;
    }
    std::uint64_t res = std::accumulate(events.begin(), events.end(), 0,
            [&graph](std::uint64_t init, VertexDescriptor vd)
            {
                const auto io_evt = boost::get<io_event_property>(graph[vd].property);
                return init + io_evt.request_size;
            });

    std::cout << "sum responses: " << res << " num events: " << events.size() << "\n";
    return static_cast<double>(res / events.size());
}

double avg_response_size(const IoGraph& graph, const std::vector<VertexDescriptor>& events)
{
    if (events.empty())
    {
        return 0.0;
    }
    std::uint64_t res = std::accumulate(events.begin(), events.end(), 0,
            [&graph](std::uint64_t init, VertexDescriptor vd)
            {
                const auto io_evt = boost::get<io_event_property>(graph[vd].property);
                return init + io_evt.response_size;
            });


    std::cout << "sum responses: " << res << " num events: " << events.size() << "\n";
    return static_cast<double>(res / events.size());
}

std::map<std::string, std::vector<VertexDescriptor>>
write_functions(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<VertexDescriptor>> write_fn_map;
    for (auto vd : cio_set)
    {
        const auto io_evt = boost::get<io_event_property>(graph[vd].property);
        if (io_evt.kind == io_event_kind::write)
        {
            write_fn_map[io_evt.region_name].push_back(vd);
        }
    }

    return write_fn_map;
}

std::map<std::string, std::vector<VertexDescriptor>>
read_functions(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<VertexDescriptor>> read_fn_map;
    for (auto vd : cio_set)
    {
        const auto io_evt = boost::get<io_event_property>(graph[vd].property);
        if (io_evt.kind == io_event_kind::read)
        {
            read_fn_map[io_evt.region_name].push_back(vd);
        }
    }

    return read_fn_map;
}

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

otf2::chrono::microseconds
get_set_duration(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    auto ts_start = graph[cio_set.start_event()].timestamp();
    auto ts_end = graph[cio_set.end_event().value()].timestamp();
    assert(ts_end > ts_start);
    auto dur_diff = ts_end - ts_start;
    auto res = otf2::chrono::duration_cast<otf2::chrono::microseconds>(dur_diff);
    return res;
}

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
        auto rw_map = region_map(graph, cio_set);
        for (const auto& kvp : rw_map)
        {
            region_stats[kvp.first] = make_rw_stats(graph, kvp.second);
        }
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


void dump_stats(const CIO_Stats& stats)
{
    const auto& read_stats = stats.get_total_read_stats();
    const auto& write_stats = stats.get_total_write_stats();
    std::cout << "Set Duration: " << stats.set_duration() << "\n";
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
}

} // namespace rabbitxx

#endif // RABBITXX_CIO_STATS_HPP

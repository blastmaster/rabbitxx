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

    std::cout << "EVENTS\n[ ";
    std::vector<std::uint64_t> event_sizes(events.size());
    std::transform(events.begin(), events.end(), event_sizes.begin(),
            [&graph](const VertexDescriptor vd)
            {
                return get_io_property(graph, vd).response_size;
            });
    std::copy(event_sizes.begin(), event_sizes.end(), std::ostream_iterator<std::uint64_t>(
                std::cout, ", "));
    std::cout << " ]\n";

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
rw_region_map(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<VertexDescriptor>> rm;
    for (auto vd : cio_set)
    {
        const auto io_evt = get_io_property(graph, vd);
        if (io_evt.kind == io_event_kind::write ||
                io_evt.kind == io_event_kind::read)
        {
            rm[io_evt.region_name].push_back(vd);
        }
    }

    return rm;
}

class CIO_Stats
{
    struct rw_stats
    {
        std::uint64_t total;
        std::uint64_t max_size;
        std::uint64_t min_size;
        double avg_size;

        rw_stats() = default;

        explicit rw_stats(std::uint64_t num_evts, std::uint64_t max_sz, std::uint64_t min_sz, double avg) noexcept
            : total(num_evts), max_size(max_sz), min_size(min_sz), avg_size(avg)
        {
        }
    };

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
        auto min_sz_it = std::min_element(events.begin(), events.end(),
                [&graph](const VertexDescriptor vd_a, const VertexDescriptor vd_b)
                {
                    const auto io_evt_a = get_io_property(graph, vd_a);
                    const auto io_evt_b = get_io_property(graph, vd_b);
                    return io_evt_a.response_size < io_evt_b.response_size;
                });
        if (min_sz_it != events.end())
        {
            min_sz = get_io_property(graph, *min_sz_it).response_size;
        }

        auto max_sz_it = std::max_element(events.begin(), events.end(),
                [&graph](const VertexDescriptor vd_a, const VertexDescriptor vd_b)
                {
                    const auto io_evt_a = get_io_property(graph, vd_a);
                    const auto io_evt_b = get_io_property(graph, vd_b);
                    return io_evt_a.response_size > io_evt_b.response_size;
                });
        if (max_sz_it != events.end())
        {
            max_sz = get_io_property(graph, *max_sz_it).response_size;
        }
        avg_sz = avg_response_size(graph, events);
        std::uint64_t size = static_cast<std::uint64_t>(events.size());
        return rw_stats(size, max_sz, min_sz, avg_sz);
    }

public:

    explicit CIO_Stats(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
    {
        std::map<io_event_kind, std::vector<VertexDescriptor>> kind_map;
        for (auto vd : cio_set)
        {
                          //TODO
            auto io_evt = get_io_property(graph, vd);
            switch (io_evt.kind)
            {
                case io_event_kind::write:
                    kind_map[io_evt.kind].push_back(vd);
                    break;
                case io_event_kind::read:
                    kind_map[io_evt.kind].push_back(vd);
                    break;
            }
        }

        write_stats = make_rw_stats(graph, kind_map[io_event_kind::write]);
        read_stats = make_rw_stats(graph, kind_map[io_event_kind::read]);
        auto rw_map = rw_region_map(graph, cio_set);
        for (const auto& kvp : rw_map)
        {
            region_stats[kvp.first] = make_rw_stats(graph, kvp.second);
        }
    }

    const rw_stats& get_read_stats() const noexcept
    {
        return read_stats;
    }

    const rw_stats& get_write_stats() const noexcept
    {
        return write_stats;
    }

    const std::map<std::string, rw_stats>&
    rw_region_stats() const noexcept
    {
        return region_stats;
    }
};


void dump_stats(const CIO_Stats& stats)
{
    const auto& read_stats = stats.get_read_stats();
    const auto& write_stats = stats.get_write_stats();
    std::cout << "Read-Stats: " << "\n" <<
        " read total: " << read_stats.total << 
        " min: " << read_stats.min_size <<
        " max: " << read_stats.max_size <<
        " avg: " << read_stats.avg_size << "\n";

    std::cout << "Write-Stats: " << "\n" <<
        " write total: " << write_stats.total << 
        " min: " << write_stats.min_size <<
        " max: " << write_stats.max_size <<
        " avg: " << write_stats.avg_size << "\n";

    std::cout << "====================" << " Read / Write - Stats per region " << "====================" << "\n";
    const auto& region_stats_map = stats.rw_region_stats();
    for (const auto& region_st : region_stats_map)
    {
        std::cout << region_st.first << " " <<
            " min: " << region_st.second.min_size <<
            " max: " << region_st.second.max_size <<
            " avg: " << region_st.second.avg_size << "\n";
    }
}

} // namespace rabbitxx

#endif // RABBITXX_CIO_STATS_HPP

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/utils.hpp>
#include <rabbitxx/log.hpp>

#include <string>
#include <iostream>

using rabbitxx::logging;

double avg_request_size(const rabbitxx::IoGraph& graph, const std::vector<rabbitxx::VertexDescriptor>& events)
{
    if (events.empty())
    {
        return 0.0;
    }
    std::uint64_t res = std::accumulate(events.begin(), events.end(), 0,
            [&graph](std::uint64_t init, rabbitxx::VertexDescriptor vd)
            {
                const auto io_evt = boost::get<rabbitxx::io_event_property>(graph[vd].property);
                return init + io_evt.request_size;
            });

    std::cout << "sum responses: " << res << " num events: " << events.size() << "\n";
    return static_cast<double>(res / events.size());
}

double avg_response_size(const rabbitxx::IoGraph& graph, const std::vector<rabbitxx::VertexDescriptor>& events)
{
    if (events.empty())
    {
        return 0.0;
    }
    std::uint64_t res = std::accumulate(events.begin(), events.end(), 0,
            [&graph](std::uint64_t init, rabbitxx::VertexDescriptor vd)
            {
                const auto io_evt = boost::get<rabbitxx::io_event_property>(graph[vd].property);
                return init + io_evt.response_size;
            });

    std::cout << "EVENTS\n[ ";
    std::vector<std::uint64_t> event_sizes(events.size());
    std::transform(events.begin(), events.end(), event_sizes.begin(),
            [&graph](const rabbitxx::VertexDescriptor vd)
            {
                return get_io_property(graph, vd).response_size;
            });
    std::copy(event_sizes.begin(), event_sizes.end(), std::ostream_iterator<std::uint64_t>(
                std::cout, ", "));
    std::cout << " ]\n";

    std::cout << "sum responses: " << res << " num events: " << events.size() << "\n";
    return static_cast<double>(res / events.size());
}

std::map<std::string, std::vector<rabbitxx::VertexDescriptor>>
write_functions(const rabbitxx::IoGraph& graph, const rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<rabbitxx::VertexDescriptor>> write_fn_map;
    for (auto vd : cio_set)
    {
        const auto io_evt = boost::get<rabbitxx::io_event_property>(graph[vd].property);
        if (io_evt.kind == rabbitxx::io_event_kind::write)
        {
            write_fn_map[io_evt.region_name].push_back(vd);
        }
    }

    return write_fn_map;
}

std::map<std::string, std::vector<rabbitxx::VertexDescriptor>>
read_functions(const rabbitxx::IoGraph& graph, const rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<rabbitxx::VertexDescriptor>> read_fn_map;
    for (auto vd : cio_set)
    {
        const auto io_evt = boost::get<rabbitxx::io_event_property>(graph[vd].property);
        if (io_evt.kind == rabbitxx::io_event_kind::read)
        {
            read_fn_map[io_evt.region_name].push_back(vd);
        }
    }

    return read_fn_map;
}

std::map<std::string, std::vector<rabbitxx::VertexDescriptor>>
rw_region_map(const rabbitxx::IoGraph& graph, const rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<rabbitxx::VertexDescriptor>> rm;
    for (auto vd : cio_set)
    {
        const auto io_evt = get_io_property(graph, vd);
        if (io_evt.kind == rabbitxx::io_event_kind::write ||
                io_evt.kind == rabbitxx::io_event_kind::read)
        {
            rm[io_evt.region_name].push_back(vd);
        }
    }

    return rm;
}

/**
 * @brief This function should determine if a read-modify-read pattern exists in
 *        the given CIO-Set.
 * @param IoGraph&
 * @param set_t<VertexDescriptor>&
 */
std::map<std::string, std::vector<rabbitxx::VertexDescriptor>>
rmw(const rabbitxx::IoGraph& graph, const rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set)
{
    std::map<std::string, std::vector<rabbitxx::VertexDescriptor>> rw_per_file;
    for (auto vd : cio_set)
    {
        const auto io_evt = boost::get<rabbitxx::io_event_property>(graph[vd].property);
        if (io_evt.kind == rabbitxx::io_event_kind::read ||
                io_evt.kind == rabbitxx::io_event_kind::write)
        {
            const std::string& file = io_evt.filename;
            rw_per_file[file].push_back(vd);
        }
    }
    return rw_per_file;
}

/**
 * creates-in-dir
 * Find all creates made within an cio-set in the same directory.
 *
 * handle, dir, operation_mode
 * 1. get cio-sets
 * 2. check `create_handle` events and check directory
 * 3. if within same cio-set and within same directory -> store
 *
 */

std::map<rabbitxx::fs::path, std::vector<rabbitxx::VertexDescriptor>>
creates_per_dir(rabbitxx::IoGraph& graph, const std::vector<rabbitxx::VertexDescriptor>& create_evts)
{
    std::map<rabbitxx::fs::path, std::vector<rabbitxx::VertexDescriptor>> creates_in_dir;
    for (const auto create_evt : create_evts)
    {
        auto io_evt = boost::get<rabbitxx::io_event_property>(graph[create_evt].property);

        rabbitxx::fs::path p(io_evt.filename);
        p.remove_filename();
        creates_in_dir[p].push_back(create_evt);
    }
    return creates_in_dir;
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
    rw_stats make_rw_stats(const rabbitxx::IoGraph& graph,
            const std::vector<rabbitxx::VertexDescriptor>& events)
    {
        std::uint64_t max_sz {0};
        std::uint64_t min_sz {0};
        double avg_sz {0.0};
        auto min_sz_it = std::min_element(events.begin(), events.end(),
                [&graph](const rabbitxx::VertexDescriptor vd_a, const rabbitxx::VertexDescriptor vd_b)
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
                [&graph](const rabbitxx::VertexDescriptor vd_a, const rabbitxx::VertexDescriptor vd_b)
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

    explicit CIO_Stats(const rabbitxx::IoGraph& graph, const rabbitxx::set_t<rabbitxx::VertexDescriptor>& cio_set)
    {
        std::map<rabbitxx::io_event_kind, std::vector<rabbitxx::VertexDescriptor>> kind_map;
        for (auto vd : cio_set)
        {
                          //TODO
            auto io_evt = get_io_property(graph, vd);
            switch (io_evt.kind)
            {
                case rabbitxx::io_event_kind::write:
                    kind_map[io_evt.kind].push_back(vd);
                    break;
                case rabbitxx::io_event_kind::read:
                    kind_map[io_evt.kind].push_back(vd);
                    break;
            }
        }

        write_stats = make_rw_stats(graph, kind_map[rabbitxx::io_event_kind::write]);
        read_stats = make_rw_stats(graph, kind_map[rabbitxx::io_event_kind::read]);
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


void print_map_stats(const rabbitxx::IoGraph& graph,
        const std::map<std::string, std::vector<rabbitxx::VertexDescriptor>>& stats)
{
    for (const auto kvp : stats)
    {
        std::cout << kvp.first << ": count: " << kvp.second.size() <<
            " avg req size: " << avg_request_size(graph, kvp.second) <<
            " avg resp size: " << avg_response_size(graph, kvp.second) << std::endl;
    }
}

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    // get cio-sets
    auto cio_sets = rabbitxx::find_cio_sets(*graph);
    int set_cnt = 0;
    for (auto& cio_set : cio_sets)
    {
        //const auto create_evts = rabbitxx::get_io_events_by_kind(*graph, cio_set, rabbitxx::io_event_kind::create);
        //auto cpd = creates_per_dir(*graph, create_evts);
        //auto rw = rmw(*graph, cio_set);
        //auto wm = write_functions(*graph, cio_set);
        std::cout << "==================== Set " << set_cnt << " ====================" << std::endl;
        CIO_Stats rw_statistic(*graph, cio_set);
        dump_stats(rw_statistic);
        //print_map_stats(*graph, wm);
        //std::cout << std::endl;
        set_cnt++;
    }

    return EXIT_SUCCESS;
}

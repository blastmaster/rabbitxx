#include <rabbitxx/cio_stats.hpp>

using namespace rabbitxx;

class RegionTimeMap
{
    using tm_type = std::map<std::string, otf2::chrono::duration>;
public:
    RegionTimeMap(const IoGraph& graph, const set_t<VertexDescriptor>& set) : set_duration_(get_set_duration(graph, set))
    {
        for (const auto evt : set)
        {
            auto r_name = graph[evt].name();
            auto it = time_map_.find(r_name);
            sum_ += graph[evt].duration.duration;
            if (it == time_map_.end())
            {
                time_map_[r_name] = graph[evt].duration.duration;
            }
            else
            {
                time_map_[r_name] += graph[evt].duration.duration;
            }
        }
    }

    const tm_type time_map() const
    {
        return time_map_;
    }

    const otf2::chrono::microseconds set_duration() const
    {
        return set_duration_;
    }

    const otf2::chrono::duration sum() const
    {
        return sum_;
    }

private:
    otf2::chrono::microseconds set_duration_;
    otf2::chrono::duration sum_ {0};
    tm_type time_map_ {};
};

class KindTimeMap
{
    using km_type = std::map<io_event_kind, otf2::chrono::duration>;
public:
    KindTimeMap(const IoGraph& graph, const set_t<VertexDescriptor>& set) : set_duration_(get_set_duration(graph, set))
    {
        for (const auto evt : set)
        {
            auto io_evt = get_io_property(graph, evt);
            auto it = kind_map_.find(io_evt.kind);
            sum_ += graph[evt].duration.duration;
            if (it == kind_map_.end())
            {
                kind_map_[io_evt.kind] = graph[evt].duration.duration;
            }
            else
            {
                kind_map_[io_evt.kind] += graph[evt].duration.duration;
            }
        }
    }

    const km_type kind_map() const
    {
        return kind_map_;
    }

    const otf2::chrono::microseconds set_duration() const
    {
        return set_duration_;
    }

    const otf2::chrono::duration sum() const
    {
        return sum_;
    }

private:
    otf2::chrono::microseconds set_duration_;
    otf2::chrono::duration sum_ {0};
    km_type kind_map_ {};
};

std::ostream& operator<< (std::ostream& os, const RegionTimeMap& rtm)
{
    //os << "set duration: " << rtm.set_duration() << "\n";
    otf2::chrono::duration d(0);
    for (const auto& kvp : rtm.time_map())
    {
        os << kvp.first << " " << kvp.second << "\n";
    }
    os << "sum i/o duration: " << rtm.sum() << "\n";
}

std::ostream& operator<< (std::ostream& os, const KindTimeMap& ktm)
{
    //os << "set duration: " << ktm.set_duration() << "\n";
    otf2::chrono::duration d(0);
    for (const auto& kvp : ktm.kind_map())
    {
        os << kvp.first << " " << kvp.second << "\n";
        d += kvp.second;
    }
    os << "sum i/o duration: " << ktm.sum() << "\n";
}

void list_event_durations(const IoGraph& graph, const set_t<VertexDescriptor>& set)
{
    for (const auto evt : set)
    {
        std::cout << "Event name: " << graph[evt].name() << " duration: " << graph[evt].duration.duration << "\n";
    }
}

void dump_region_time_map(const IoGraph& graph, const set_container_t<VertexDescriptor>& sets)
{
    std::vector<RegionTimeMap> rtm_per_set;
    std::transform(sets.begin(), sets.end(), std::back_inserter(rtm_per_set),
            [&graph](const set_t<VertexDescriptor>& set)
            {
                return RegionTimeMap(graph, set);
            });

    std::copy(rtm_per_set.begin(), rtm_per_set.end(),
            std::ostream_iterator<RegionTimeMap>(std::cout, "\n"));
}

void dump_kind_time_map(const IoGraph& graph, const set_container_t<VertexDescriptor>& sets)
{
    std::vector<KindTimeMap> ktm_per_set;
    std::transform(sets.begin(), sets.end(), std::back_inserter(ktm_per_set),
            [&graph](const set_t<VertexDescriptor>& set)
            {
                return KindTimeMap(graph, set);
            });

    std::copy(ktm_per_set.begin(), ktm_per_set.end(),
            std::ostream_iterator<KindTimeMap>(std::cout, "\n"));
}

void dump_csv(const IoGraph& graph, const set_container_t<VertexDescriptor>& sets)
{
    const std::array<std::string, 9> head {
        "set",
        "create",
        "dup",
        "seek",
        "read",
        "write",
        "flush",
        "close/delete",
        "sum"
    };
    const std::array<io_event_kind, 7> kinds {
        io_event_kind::create,
        io_event_kind::dup,
        io_event_kind::seek,
        io_event_kind::read,
        io_event_kind::write,
        io_event_kind::flush,
        io_event_kind::delete_or_close
    };
    std::copy(head.begin(), head.end(), std::ostream_iterator<std::string>(std::cout, ","));
    std::cout << "\n";
    std::uint64_t sidx {1};
    for (const auto& set : sets)
    {
        KindTimeMap cur(graph, set);
        std::cout << sidx << ", ";
        for (const auto& kind : kinds)
        {
            auto km = cur.kind_map();
            if (km.find(kind) != km.end())
            {
                std::cout << km[kind] << ", ";
            }
            else
            {
                std::cout << otf2::chrono::picoseconds(0) << ", ";
            }
        }
        std::cout << cur.sum() << "\n";
        ++sidx;
    }
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    // create graph
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(argv[1]);
    // find concurrent I/O-Sets
    auto io_sets = find_cio_sets(graph);

    // just dump all durations
    //for (const auto& set : io_sets)
    //{
        //list_event_durations(graph, set);
    //}

    //dump_region_time_map(graph, io_sets);
    //std::cout << "\n\n";
    //dump_kind_time_map(graph, io_sets);
    dump_csv(graph, io_sets);

    return EXIT_SUCCESS;
}

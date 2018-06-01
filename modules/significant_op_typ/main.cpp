#include <rabbitxx/cio_stats.hpp>

using namespace rabbitxx;

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
    dump_csv(graph, io_sets);

    return EXIT_SUCCESS;
}

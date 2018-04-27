#include <rabbitxx/log.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/cio_stats.hpp>

#include <iostream>

using namespace rabbitxx;

/**
 * raw size: 53[DEBUG]: w/o empty sets: 13[DEBUG]: unique sets: 13Number of Cio-Sets: 13
 * Sum of Events in Cio-Sets: 346292
 * Set Sizes: 94770, 71092, 59239, 47374, 35533, 23686, 11847, 736, 656, 646, 10, 642, 61,
**/

std::set<std::string>
region_names_by_io_event_kind(const IoGraph& graph,
        const std::vector<VertexDescriptor>& events)
{
    std::set<std::string> names;
    for (VertexDescriptor vd : events)
    {
        names.insert(graph[vd].name());
    }

    return names;
}

void print_stats_per_io_event_kind(const IoGraph& graph,
        const std::vector<VertexDescriptor>& events,
        const io_event_kind kind)
{
    std::cout << "Number of " << kind << "-events: " << events.size() << std::endl;
    const auto io_funcs = region_names_by_io_event_kind(graph, events);
    std::cout << kind << "-funcs: [ ";
    std::copy(io_funcs.begin(), io_funcs.end(), std::ostream_iterator<std::string>(std::cout, ", "));
    std::cout << " ]" << std::endl;
}

void print_single_cio_set_stats(const IoGraph& graph,
        const set_t<VertexDescriptor>& cio_set)
{
    CIO_Stats rw_statistic(graph, cio_set);
    auto num_events = cio_set.size();
    auto io_evt_kind_map = kind_map(graph, cio_set);
    std::cout << "Number of events in Cio-Set: " << num_events << std::endl;
    for (auto kvp : io_evt_kind_map)
    {
        print_stats_per_io_event_kind(graph, kvp.second, kvp.first);
    }
    auto sum_evts = std::accumulate(io_evt_kind_map.begin(), io_evt_kind_map.end(), 0,
            [](const int& init, const auto& kvp) {
                return init + kvp.second.size();
            });
    assert(sum_evts == num_events);
    dump_stats(rw_statistic);
    std::cout << std::endl;
}

void print_cio_set_stats(const IoGraph& graph,
    const set_container_t<VertexDescriptor>& cio_sets)
{
    auto n_sets = cio_sets.size();
    std::vector<VertexDescriptor> set_sizes(n_sets);
    std::transform(cio_sets.begin(), cio_sets.end(), set_sizes.begin(),
            [](const auto& cio_set) {
                return cio_set.size();
            });

    std::cout << "Number of Cio-Sets: " << n_sets << std::endl;
    std::cout << "Sum of Events in Cio-Sets: "
        << std::accumulate(set_sizes.begin(), set_sizes.end(), 0)
        << std::endl;
    std::cout << "Set Sizes: ";
    std::copy(set_sizes.begin(), set_sizes.end(),
            std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
    std::cout << std::endl;
    std::cout << "==================== Stats per Cio-Set ====================" << std::endl;
    int set_cnt {0};
    for (const auto& cio_set : cio_sets)
    {
        std::cout << "==================== Set " << set_cnt << " ====================" << std::endl;
        print_single_cio_set_stats(graph, cio_set);
        std::cout << std::endl;
        ++set_cnt;
    }
}

void print_cio_set_per_process_stats(IoGraph& graph,
        set_map_t<VertexDescriptor> sets_per_proc)
{
    int cnt {0};
    remove_empty_sets(sets_per_proc); //Thats the reason for taking the parameter by value!
    std::cout << "==================== Cio-Set per process stats ====================" << std::endl;
    for (const auto& set_pp_kvp : sets_per_proc)
    {
        int cnt_pp = set_pp_kvp.second.size();
        std::cout << "Number of CIO-Sets per process [" << set_pp_kvp.first << "] " << cnt_pp << std::endl;
        cnt += cnt_pp;
    }
    std::cout << "Sum CIO-Sets per process: " << cnt << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: " << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trc_file);
    // get cio-sets per process
    auto cio_sets_pp = cio_sets_per_process(graph);
    print_cio_set_per_process_stats(graph, cio_sets_pp);
    // get cio-sets
    auto cio_sets = find_cio_sets(graph, cio_sets_pp);
    print_cio_set_stats(graph, cio_sets);

    return EXIT_SUCCESS;
}

#include <rabbitxx/cio_set.hpp>

#include <algorithm>
#include <numeric>

namespace rabbitxx
{

// set-api
void
remove_empty_sets(set_map_t<VertexDescriptor>& sets)
{
    std::for_each(
        sets.begin(), sets.end(), [](auto& proc_sets) { remove_empty_sets(proc_sets.second); });
}

void
remove_empty_sets(set_container_t<VertexDescriptor>& sets)
{
    sets.erase(
        std::remove_if(sets.begin(), sets.end(), [](const auto& set) { return set.empty(); }),
        sets.end());
}

/**
 * TODO: eventually we could use a set as container and get rid of the sort and
 * the erase + unique too.
 */
std::vector<VertexDescriptor>
collect_root_sync_events(IoGraph& graph)
{
    using vertex_descriptor = VertexDescriptor;
    std::vector<vertex_descriptor> result;
    // get all sync and synthetic events in the graph
    const auto sync_events =
        get_events_by_kind(graph, { vertex_kind::sync_event, vertex_kind::synthetic });
    // store the root-sync-event in result vector
    std::transform(sync_events.begin(), sync_events.end(), std::back_inserter(result),
        [&graph](const vertex_descriptor& vd) {
            if (graph[vd].type == vertex_kind::sync_event)
            {
                return root_of_sync(vd, *graph.get());
            }
            return vd;
        });
    // sort the events if necessary
    if (!std::is_sorted(result.begin(), result.end()))
    {
        std::sort(result.begin(), result.end());
    }
    // delete everything but hold unique ones
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

// set-api
map_view_t<VertexDescriptor>
make_mapview(set_map_t<VertexDescriptor>& smap)
{
    map_view_t<VertexDescriptor> m_v;
    for (auto& setmap_kvp : smap)
    {
        auto iter_p = std::make_pair(setmap_kvp.second.begin(), setmap_kvp.second.end());
        m_v.insert(std::make_pair(setmap_kvp.first, iter_p));
    }

    return m_v;
}

// set-api
map_view_t<VertexDescriptor>
update_view(const process_group_t& pg, map_view_t<VertexDescriptor> map_view)
{
    for (const auto& proc : pg)
    {
        const auto& dbg_cached_value = map_view[proc].first->end_event();
        map_view[proc].first = std::next(map_view[proc].first);
        //logging::debug() << "process: " << proc << " increment iterator from " << dbg_cached_value
                         //<< " to " << map_view[proc].first->end_event();
    }

    return map_view;
}

//TODO: needed?!?
pg_map_t<VertexDescriptor>
make_local_pgmap(IoGraph& graph)
{
    using vertex_descriptor = VertexDescriptor;
    pg_map_t<vertex_descriptor> pg_map;
    const auto sync_evt_roots_v = collect_root_sync_events(graph);
    // needs to be sorted?!?
    for (const auto& vd : sync_evt_roots_v)
    {
        if (graph[vd].type == vertex_kind::sync_event)
        {
            const auto evt_property = boost::get<sync_event_property>(graph[vd].property);
            const auto inv_proc_v = procs_in_sync_involved(evt_property);
            const auto s_scope = classify_sync(graph, evt_property);
            if (s_scope == sync_scope::Local)
            {
                pg_map.insert(
                    std::make_pair(vd, process_group_t(inv_proc_v.begin(), inv_proc_v.end())));
            }
        }
        else
        {
            logging::fatal() << "not a sync event: " << vd << " in make_local_pg_map";
        }
    }

    return pg_map;
}

// set-api
process_group_t
pg_group(const IoGraph& graph, const VertexDescriptor& vd)
{
    if (graph[vd].type == vertex_kind::sync_event)
    {
        const auto& evt_property = get_sync_property(graph, vd);
        const auto& inv_proc_v = procs_in_sync_involved(evt_property);
        const auto& scope = classify_sync(graph, evt_property);
        if (scope == sync_scope::Local)
        {
            //logging::debug() << "Retrun process_group_t of LOCAL sync-event: " << vd;
            return process_group_t(inv_proc_v.begin(), inv_proc_v.end());
        }
        if (scope == sync_scope::Global)
        {
            //logging::debug() << "Retrun process_group_t of GLOBAL sync-event: " << vd;
            return process_group_t(inv_proc_v.begin(), inv_proc_v.end());
        }
        logging::fatal() << "undefined sync_scope not handled! Event: " << vd;
        throw - 1; // TODO: proper error handling!
    }
    if (graph[vd].type == vertex_kind::synthetic)
    {
        //logging::debug() << "Retrun process_group_t of GLOBAL synthetic-event: " << vd;
        const auto np = num_procs(graph);
        std::vector<std::uint64_t> all_procs(np);
        std::iota(all_procs.begin(), all_procs.end(), 0);
        return process_group_t(all_procs.begin(), all_procs.end());
    }

    return {};
}

std::vector<VertexDescriptor>
// IoGraph const?!?
do_merge(const IoGraph& graph, const map_view_t<VertexDescriptor>& map_view,
    std::vector<set_t<VertexDescriptor>>& merged_sets)
{
    std::vector<VertexDescriptor> end_evts;
    set_t<VertexDescriptor> cur_s;
    for (const auto& mv_kvp : map_view)
    {
        const auto& first_set = *mv_kvp.second.first;
        cur_s.merge(first_set);
        end_evts.push_back(first_set.end_event().value());
    }

    // JUST FOR DEBUGGING
    //std::cout << "END-EVENTS:\n";
    //std::copy(
        //end_evts.begin(), end_evts.end(), std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
    //std::cout << "\n";

    const auto e_evts = detail::find_end_events_to_update(graph, end_evts);
    if (!e_evts.empty())
    {
        cur_s.close();
        //TODO: This is a Hack, why use the back?
        cur_s.set_end_event(e_evts.back());
        merged_sets.push_back(cur_s);
    }
    else
    {
        logging::fatal() << "ERROR NO END EVENT FOUND TO UPDATE! THIS SHOULD NOT HAPPEN!";
        throw -1; //TODO
    }

    return e_evts;
}

void
process_sets(const IoGraph& graph, map_view_t<VertexDescriptor> map_view,
    std::vector<set_t<VertexDescriptor>>& merged_sets)
{
    bool on_end = std::all_of(map_view.begin(), map_view.end(),
        [](const auto& p_iters) { return p_iters.second.first == p_iters.second.second; });
    if (on_end)
    {
        return;
    }

    const auto end_events = do_merge(graph, map_view, merged_sets);

    for (const auto& end_evt : end_events)
    {
        //logging::debug() << "Choose end-event: " << end_evt << " -> recursive update!";
        const auto lpg = pg_group(graph, end_evt);
        process_sets(graph, update_view(lpg, map_view), merged_sets);
    }
}

inline set_container_t<VertexDescriptor>
merge_sets_impl(const IoGraph& graph, set_map_t<VertexDescriptor>& set_map)
{
    set_container_t<VertexDescriptor> merged_sets;
    auto map_view = make_mapview(set_map);
    assert(map_view.size() == set_map.size());
    process_sets(graph, map_view, merged_sets);

    return merged_sets;
}

set_container_t<VertexDescriptor>
merge_sets(const IoGraph& graph, set_map_t<VertexDescriptor>& set_map)
{
    return merge_sets_impl(graph, set_map);
}

/**
 * Here we get the Sets per process - nothing is merged together!
 */
set_map_t<VertexDescriptor>
cio_sets_per_process(IoGraph& graph)
{
    using map_t = set_map_t<VertexDescriptor>;

    auto root = find_root(graph);
    assert(graph[root].type == vertex_kind::synthetic);
    auto shared_set_container(std::make_shared<map_t>());
    CIO_Visitor<map_t> vis(shared_set_container);
    std::vector<boost::default_color_type> color_map(graph.num_vertices());
    boost::depth_first_visit(*graph.get(), root, vis,
        make_iterator_property_map(color_map.begin(), get(boost::vertex_index, *graph.get())));
    detail::sort_set_map_chrono(graph, *shared_set_container.get());

    return *shared_set_container.get();
}

// all-in-one version - do merging
set_container_t<VertexDescriptor>
find_cio_sets(IoGraph& graph)
{
    using map_t = set_map_t<VertexDescriptor>;

    map_t sets_per_process = cio_sets_per_process(graph);
    return find_cio_sets(graph, sets_per_process);
}

set_container_t<VertexDescriptor>
find_cio_sets(const IoGraph& graph, set_map_t<VertexDescriptor>& sets_per_process)
{
    auto merged_sets = merge_sets(graph, sets_per_process);
    //logging::debug() << "Resulting Sets:\n" << "raw size: " << merged_sets.size();
    // remove empty sets
    remove_empty_sets(merged_sets);
    //logging::debug() << "w/o empty sets: " << merged_sets.size();
    // sort and remove duplicates
    std::sort(merged_sets.begin(), merged_sets.end());
    merged_sets.erase(std::unique(merged_sets.begin(), merged_sets.end()), merged_sets.end());

    return merged_sets;
}

std::vector<VertexDescriptor>
get_io_events_by_kind(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set, io_event_kind kind)
{
    std::vector<VertexDescriptor> result;
    std::copy_if(cio_set.begin() , cio_set.end(), std::back_inserter(result),
            [&graph, &kind](const VertexDescriptor& vd) {
                return boost::get<io_event_property>(graph[vd].property).kind == kind;
            });
    return result;
}

std::vector<VertexDescriptor>
get_io_events_by_kind(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set, const std::vector<io_event_kind>& kinds)
{
    std::vector<VertexDescriptor> results;
    std::copy_if(cio_set.begin(), cio_set.end(), std::back_inserter(results),
            [&graph, &kinds](const VertexDescriptor& vd) {
                const auto io_evt_kind = boost::get<io_event_property>(graph[vd].property).kind;
                return std::any_of(kinds.begin(), kinds.end(),
                        [&io_evt_kind](const io_event_kind kind) {
                            return io_evt_kind == kind;
                        });
            });
    return results;
}

namespace detail
{

/**
 * XXX: The assert will fail if used on final sets! Since there is no origin.
 * This is not a final solution, it should work since events of the same rank
 * can not overhaul themselve.
 * TODO: Nevertheless, sorting the sets using a adjacency relation might be
 * better.
 */
void
sort_set_map_chrono(const IoGraph& graph, set_map_t<VertexDescriptor>& set_map)
{
    auto chrono_cmp = [&graph](const set_t<VertexDescriptor>& set_a,
        const set_t<VertexDescriptor>& set_b) {
        const auto vd_a = set_a.origin();
        const auto vd_b = set_b.origin();
        assert(vd_a != std::numeric_limits<decltype(vd_a)>::max());
        assert(vd_b != std::numeric_limits<decltype(vd_b)>::max());
        const auto t_a = graph[vd_a].timestamp();
        const auto t_b = graph[vd_b].timestamp();
        return t_a < t_b;
    };

    for (auto& proc_sets : set_map)
    {
        std::sort(proc_sets.second.begin(), proc_sets.second.end(), chrono_cmp);
    }
}

// number of combinations of unique end-evt-pairs n(n-1)/2
std::size_t
num_unique_pairs(const std::size_t n)
{
    return n * (n - 1) / 2;
}

// O(n^2)
std::vector<std::pair<VertexDescriptor, VertexDescriptor>>
generate_unique_pairs(const std::vector<VertexDescriptor>& v)
{
    using vd_t = VertexDescriptor;
    std::vector<std::pair<vd_t, vd_t>> res;
    for (vd_t i = 0ul; i < v.size(); ++i)
    {
        vd_t t1 = v[i];
        for (vd_t j = i + 1ul; j < v.size(); ++j)
        {
            vd_t t2 = v[j];
            res.push_back(std::make_pair(t1, t2));
        }
    }

    return res;
}

bool
can_update_end_event(const process_group_t& pgroup, const std::vector<VertexDescriptor>& end_evts,
    const VertexDescriptor& pivot)
{
    return std::all_of(pgroup.begin(), pgroup.end(),
        [&end_evts, &pivot](const std::uint64_t pid) { return end_evts[pid] == pivot; });
}

bool
can_update_end_event(
    const IoGraph& graph, const std::vector<VertexDescriptor>& end_evts, const VertexDescriptor& pivot)
{
    const auto pgroup = pg_group(graph, pivot);
    return can_update_end_event(pgroup, end_evts, pivot);
}

std::vector<VertexDescriptor>
find_end_events_to_update(const IoGraph& graph, std::vector<VertexDescriptor> end_evts)
{
    auto check_update_func = [end_evts](const IoGraph& graph, const std::set<VertexDescriptor>& try_s) {
        for (const VertexDescriptor& vd : try_s)
        {
            bool update = can_update_end_event(graph, end_evts, vd);
            if (update)
            {
                return vd;
            }
        }
    };

    auto check_update_func_single = [end_evts](const IoGraph& graph, const VertexDescriptor& vd) {
        return can_update_end_event(graph, end_evts, vd);
    };

    // sort
    std::sort(end_evts.begin(), end_evts.end());
    // remove duplicates
    end_evts.erase(std::unique(end_evts.begin(), end_evts.end()), end_evts.end());

    // just one unique sync-event need to be global! Return it!
    if (end_evts.size() == 1)
    {
        assert(sync_scope::Global == classify_sync(graph, end_evts.back()));
        // TODO: check if can update
        return end_evts;
    }

    // not global case! Remove all global syncs;
    end_evts.erase(std::remove_if(end_evts.begin(), end_evts.end(),
                       [&graph](const auto& evt) {
                           const auto scope = classify_sync(graph, evt);
                           return scope == sync_scope::Global;
                       }),
        end_evts.end());

    if (end_evts.size() == 1)
    {
        assert(sync_scope::Local == classify_sync(graph, end_evts.back()));
        //logging::debug() << "JUST ONE LOCAL EVENT SO JUST RETURN!";
        // TODO: check if can update
        return end_evts;
    }

    // we have more than one end-event with sync_scope::Local
    const auto upairs_v = generate_unique_pairs(end_evts);
    assert(upairs_v.size() == num_unique_pairs(end_evts.size()));
    std::set<VertexDescriptor> independent_syncs, dependent_syncs;
    std::vector<std::uint64_t> overlapping_procs;
    for (const auto& sync_p : upairs_v)
    {
        const auto pg1 = pg_group(graph, sync_p.first);
        const auto pg2 = pg_group(graph, sync_p.second);

        std::vector<std::uint64_t> intersection_procs;
        std::set_intersection(
            pg1.begin(), pg1.end(), pg2.begin(), pg2.end(), std::back_inserter(intersection_procs));

        if (intersection_procs.empty())
        {
            //logging::debug() << "independent sync pair: (" << sync_p.first << ", " << sync_p.second
                             //<< ")";
            independent_syncs.insert(sync_p.first);
            independent_syncs.insert(sync_p.second);
        }
        else
        {
            //logging::debug() << "overlapping procs from depending sync pair: "
                             //<< "(" << sync_p.first << ", " << sync_p.second << ")";
            dependent_syncs.insert(sync_p.first);
            dependent_syncs.insert(sync_p.second);
            std::copy(intersection_procs.begin(), intersection_procs.end(),
                std::back_inserter(overlapping_procs));
        }
    }

    // JUST FOR DEBUGGING
    //logging::debug() << "OVERLAPPING PROCS:";
    //std::copy(overlapping_procs.begin(), overlapping_procs.end(),
        //std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
    //std::cout << "\n";

    VertexDescriptor update_evt;
    // first check dependent events
    if (!dependent_syncs.empty())
    {
        //logging::debug() << "choose from dependent syncs";
        update_evt = check_update_func(graph, dependent_syncs);
    }
    // afterwards check for independent events
    else if (!independent_syncs.empty())
    {
        //logging::debug() << "choose from independent syncs";
        // update_evt = check_update_func(graph, independent_syncs);
        std::vector<VertexDescriptor> res;
        for (const auto& isv : independent_syncs)
        {
            if (check_update_func_single(graph, isv))
            {
                res.push_back(isv);
            }
        }
        return res;
    }
    else
    {
        logging::fatal() << "ERROR NO END EVENT FOUND !";
        throw - 1; //TODO
    }

    return { update_evt };
}

} // namespace detail

} // namespace rabbitxx

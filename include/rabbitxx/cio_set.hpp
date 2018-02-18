#ifndef RABBITXX_CIO_SET_HPP
#define RABBITXX_CIO_SET_HPP

#include <rabbitxx/log.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_types.hpp>

#include <algorithm>
#include <numeric>

namespace rabbitxx
{

// set-api
void remove_empty_sets(set_map_t<VertexDescriptor>& sets);

void remove_empty_sets(set_container_t<VertexDescriptor>& sets);

/**
 * TODO: eventually we could use a set as container and get rid of the sort and
 * the erase + unique too.
 */
std::vector<VertexDescriptor> collect_root_sync_events(IoGraph& graph);

// set-api
map_view_t<VertexDescriptor> make_mapview(set_map_t<VertexDescriptor>& smap);

// set-api
map_view_t<VertexDescriptor> update_view(const process_group_t& pg, map_view_t<VertexDescriptor> map_view);


//TODO: needed?!?
pg_map_t<VertexDescriptor> make_local_pgmap(IoGraph& graph);

// set-api
process_group_t pg_group(IoGraph& graph, const VertexDescriptor& vd);


/**
 * TODO: Here, we do two things, we merge the current view of the per process
 * set_map into a new set. Afterwards we need to close the new set and setting a
 * new `end_evt` before. Therefore we also need to find the new `end_event` to
 * set.
 *
 * The problem is that we need this `end_event` also later, to decide how to
 * update the map. But we also need the end_event vector, we already return.
 * One option is to return a pair of end_event and vector.
 * A Second could be the creation of a separat type.
 *
 * If I Implement a `find_end_event` function it need to find the indendent sync
 * pairs anyway so it could be possible that the end_event later is no longer
 * needed.
 */
std::vector<VertexDescriptor>
do_merge(IoGraph& graph, const map_view_t<VertexDescriptor>& map_view,
    std::vector<set_t<VertexDescriptor>>& merged_sets);

void
process_sets(IoGraph& graph, map_view_t<VertexDescriptor> map_view,
    std::vector<set_t<VertexDescriptor>>& merged_sets);

inline set_container_t<VertexDescriptor>
merge_sets_impl(IoGraph& graph, set_map_t<VertexDescriptor>& set_map);

set_container_t<VertexDescriptor>
merge_sets(IoGraph& graph, set_map_t<VertexDescriptor>& set_map);

/**
 * Here we get the Sets per process - nothing is merged together!
 */
set_map_t<VertexDescriptor> cio_sets_per_process(IoGraph& graph);

// all-in-one version - do merging
set_container_t<VertexDescriptor> find_cio_sets(IoGraph& graph);

// find final cio sets with cio_sets per process given
set_container_t<VertexDescriptor> find_cio_sets(IoGraph& graph,
        set_map_t<VertexDescriptor>& cio_set_pp);

//FIXME ambigious overload, same function is already defined in `io_graph` 
//but just works on graph instead of cio_sets.
std::vector<VertexDescriptor> get_io_events_by_kind(const IoGraph& graph,
        const set_t<VertexDescriptor>& cio_set,
        io_event_kind kind);

std::vector<VertexDescriptor>
get_io_events_by_kind(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set, const std::vector<io_event_kind>& kinds);

namespace detail
{

// TODO: should at least be hidden from accedential use
/**
 * XXX: The assert will fail if used on final sets! Since there is no origin.
 * This is not a final solution, it should work since events of the same rank
 * can not overhaul themselve.
 * TODO: Nevertheless, sorting the sets using a adjacency relation might be
 * better.
 */
void
sort_set_map_chrono(const IoGraph& graph, set_map_t<VertexDescriptor>& set_map);

// number of combinations of unique end-evt-pairs n(n-1)/2
std::size_t
num_unique_pairs(const std::size_t n);

// O(n^2)
std::vector<std::pair<VertexDescriptor, VertexDescriptor>>
generate_unique_pairs(const std::vector<VertexDescriptor>& v);

bool
can_update_end_event(const process_group_t& pgroup, const std::vector<VertexDescriptor>& end_evts,
    const VertexDescriptor& pivot);

bool
can_update_end_event(
    IoGraph& graph, const std::vector<VertexDescriptor>& end_evts, const VertexDescriptor& pivot);

std::vector<VertexDescriptor>
find_end_events_to_update(IoGraph& graph, std::vector<VertexDescriptor> end_evts);

} // namespace detail

} // namespace rabbitxx

#endif /* RABBITXX_CIO_SET_HPP */

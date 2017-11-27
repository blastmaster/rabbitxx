#ifndef RABBITXX_CIO_SET_HPP
#define RABBITXX_CIO_SET_HPP

#include <rabbitxx/log.hpp>

#include <boost/graph/depth_first_search.hpp>
#include <boost/optional.hpp>

#include <map>
#include <unordered_set>
#include <vector>
#include <algorithm>

namespace rabbitxx {

enum class Set_State
{
    Open,
    Closed
};

std::ostream& operator<<(std::ostream& os, const Set_State state)
{
    switch (state)
    {
        case Set_State::Open:
            os << "Open"; break;
        case Set_State::Closed:
            os << "Closed"; break;
    }
    return os;
}

template<typename Set>
using ProcSet = std::vector<Set>;

template<typename ProcessSet>
using ProcSetMap = std::map<std::uint64_t, ProcessSet>;

/**
 *
 * @tparam VertexDescriptor must be an integral type
 */
template<typename VertexDescriptor>
class CIO_Set
{
    public:
    using value_type = VertexDescriptor;
    //using set_t = std::unordered_set<value_type>;
    using set_t = std::set<value_type>;
    using size_type = typename set_t::size_type;
    using iterator = typename set_t::iterator;
    using const_iterator = typename set_t::const_iterator;

    CIO_Set() = default;

    CIO_Set(const value_type& start_event) 
        : start_evt_(start_event), state_(Set_State::Open)
    {
    }

    CIO_Set(const CIO_Set<value_type>& other) = default;

    void merge(const CIO_Set<value_type>& other_set)
    {
        // choose earliest start_evt_ of all sets merged into the current.
        start_evt_ = other_set.start_event() < start_evt_ ? other_set.start_event() : start_evt_;
        if (other_set.empty()) {
            logging::debug() << "in merge, other set is empty... skip!";
            return;
        }

        std::copy(other_set.begin(), other_set.end(),
                std::inserter(set_, set_.begin()));
    }

    Set_State state() const noexcept
    {
        return state_;
    }

    set_t set() const noexcept
    {
        return set_;
    }

    void close() noexcept
    {
        state_ = Set_State::Closed;
    }

    value_type start_event() const noexcept
    {
        return start_evt_;
    }

    boost::optional<value_type> end_event() const noexcept
    {
        return end_evt_;
    }

    void set_end_event(const value_type& end_event)
    {
        end_evt_ = end_event;
    }

    size_type size() const noexcept
    {
        return set_.size();
    }

    bool empty() const noexcept
    {
        return set_.empty();
    }

    auto insert(const value_type& value)
    {
        return set_.insert(value);
    }

    iterator begin() noexcept
    {
        return set_.begin();
    }

    iterator end() noexcept
    {
        return set_.end();
    }

    const_iterator begin() const noexcept
    {
        return set_.begin();
    }

    const_iterator end() const noexcept
    {
        return set_.end();
    }

    const_iterator cbegin() const noexcept
    {
        return set_.begin();
    }

    const_iterator cend() const noexcept
    {
        return set_.cend();
    }

    private:
    value_type start_evt_ = std::numeric_limits<value_type>::max();
    Set_State state_ = Set_State::Open;
    boost::optional<value_type> end_evt_ = boost::none;
    set_t set_ {};
};

template<typename DescriptorType>
inline std::ostream& operator<<(std::ostream& os, const CIO_Set<DescriptorType>& set)
{
    os << "CIO_Set {\n"
        << "\t[State] " << set.state() << "\n"
        << "\t[Start Evt] " << set.start_event() << "\n";
    if (boost::optional<DescriptorType> end_evt = set.end_event()) {
        os << "\t[End Evt] " << *end_evt << "\n";
    }
    else {
        os << "\t[End Evt] " << "NONE\n";
    }
    os << "\t[Events] [ ";
    std::copy(set.begin(), set.end(), 
            std::ostream_iterator<DescriptorType>(os, ", "));
    os << " ]\n};\n";
    return os;
}

/**
 * @brief Get all in-going synchronization-events of a given vertex.
 *
 * @param v: The vertex descriptor of the synchronization-event.
 * @param g: A Reference to the Graph.
 *
 * @return Retruns a vector containing the vertex descriptors of all in-going
 * synchronization events from a given vertex v.
 */
template<typename Vertex, typename Graph>
std::vector<Vertex>
get_in_going_syncs(Vertex v, Graph& g)
{
    const auto in_edge_r = boost::in_edges(v, g);
    std::vector<Vertex> results;
    for (auto edge_it = in_edge_r.first; edge_it != in_edge_r.second; ++edge_it) {
        const auto src_v = source(*edge_it, g);
        if (vertex_kind::sync_event == g[src_v].type) {
            results.push_back(src_v);
        }
    }
    return results;
}

/**
 * @brief Find the root-Event of a given Synchronization Event.
 * 
 * @param v the vertex descriptor of a synchronization-event.
 * @param g a reference to the graph.
 *
 * @return The vertex descriptor of the root-event of the synchronization.
 */
template<typename Vertex, typename Graph>
Vertex root_of_sync(Vertex v, Graph& g)
{
    assert(vertex_kind::sync_event == g[v].type);
    const auto in_dgr = boost::in_degree(v, g);
    if (in_dgr == 1) {
        return v;
    }
    const auto sync_evt = boost::get<sync_event_property>(g[v].property);
    // if p2p event get remote proc
    if (sync_evt.comm_kind == sync_event_kind::p2p) {
        const auto p2p_evt = boost::get<rabbitxx::peer2peer>(sync_evt.op_data);
        const auto r_proc = p2p_evt.remote_process();
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, g);
        for (const auto s : in_syncs) {
            const auto id = g[s].id();
            if (id == r_proc) { // Test if r_proc is proc_id from root!
                logging::debug() << "process id matches remote proc id!";
                return s;
            }
        }
    } // if collective
    else if (sync_evt.comm_kind == sync_event_kind::collective) {
        const auto coll_evt = boost::get<collective>(sync_evt.op_data);
        // check if collective has an explicit root
        if (coll_evt.has_root()) {
            const auto root_rank = coll_evt.root();
            if (root_rank == g[v].id()) { // we *are* the root sync event!
                logging::debug() << " v " << v << " root rank " << root_rank;
                return v; // return our own vertex descriptor
            }
        }
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, g);
        for (const auto s : in_syncs) {
            const auto in_dgr = boost::in_degree(s, g);
            const auto out_dgr = boost::out_degree(s, g);
            if ((out_dgr >= coll_evt.members().size() - 1) && (in_dgr == 1)) {
                return s;
            }
        }
    }
}

/**
 * @brief Finds the synthetic root node of a given graph.
 *
 * Root node is the only vertex with a `vertex_kind::synthetic`.
 *
 * @param graph: A reference to the Graph.
 * @return The vertex descriptor of the root node.
 */
template<typename Graph>
typename Graph::vertex_descriptor
find_root(Graph& graph)
{
    const auto vertices = graph.vertices();
    const auto root = std::find_if(vertices.first, vertices.second,
            [&graph](const typename Graph::vertex_descriptor& vd) {
                return graph[vd].type == vertex_kind::synthetic;
            });
    return *root;
}

/**
 * @brief Get the number of processes involved, in a given I/O Graph.
 * 
 * Therefore we take the synthetic root vertex and count the out-going edges.
 *
 * @param graph: A reference to the graph.
 * @return The number of processes that have events in the graph.
 */
template<typename Graph>
std::uint64_t num_procs(Graph& graph) noexcept
{
    const auto root = find_root(graph);
    return static_cast<std::uint64_t>(boost::out_degree(root, *graph.get()));
}

/**
 * @brief Get all processes which are involved in a given synchronization event.
 *
 * Return vector containing all the process id's of the processes participating
 * within a given synchronization event.
 *
 * @param sevt: Reference to a synchronization event.
 * @return Vector of process ids, which are involved in the synchronization.
 */
std::vector<std::uint64_t>
procs_in_sync_involved(const sync_event_property& sevt)
{
    if (sevt.comm_kind == sync_event_kind::collective) {
        const auto coll_evt = boost::get<collective>(sevt.op_data);
        return coll_evt.members();
    }
    else if (sevt.comm_kind == sync_event_kind::p2p) {
        const auto p2p_evt = boost::get<peer2peer>(sevt.op_data);
        return {sevt.proc_id, p2p_evt.remote_process()};
    }
}

/**
 * Return the number of processes involved in a given synchronization routine.
 */
std::uint64_t num_procs_in_sync_involved(const sync_event_property& sevt)
{
    return procs_in_sync_involved(sevt).size();
}


template<typename Graph, typename Vertex, typename Visitor>
void traverse_adjacent_vertices(Graph& graph, Vertex v, Visitor& vis)
{
    typename Graph::adjacency_iterator adj_begin, adj_end;
    for (std::tie(adj_begin, adj_end) = boost::adjacent_vertices(v, *graph.get());
         adj_begin != adj_end;
        ++adj_begin)
    {
       vis(graph, *adj_begin);
       traverse_adjacent_vertices(graph, *adj_begin, vis);
    }
}

/**
* Checks if the start event of a set `of_v` is an adjacent vertex of the current
* event `cur_v`.
*/
template<typename Vertex, typename Graph>
bool is_adjacent_event_of(const Vertex of_v, const Vertex cur_v, const Graph& g)
{
    const auto adjacent_r = boost::adjacent_vertices(cur_v, g);
    return std::any_of(adjacent_r.first, adjacent_r.second,
            [&of_v](const Vertex vd) { return vd == of_v; });
}

template<typename Cont>
class CIO_Visitor : public boost::default_dfs_visitor
{
    public:
        CIO_Visitor(std::shared_ptr<Cont>& sp) : set_cnt_ptr_(sp)
        {
        }

        template<typename Vertex, typename Graph>
        void discover_vertex(Vertex v, const Graph& g)
        {
            const auto cur_pid = g[v].id();
            auto *set_ptr = find_open_set_for(cur_pid);
            if (set_ptr == nullptr) { // no open set was found for this process
                logging::debug() << "No open set was found!";
                if (vertex_kind::io_event == g[v].type) {
                    logging::fatal() << "I/O Event ... THIS SHOULD NEVER HAPPEN";
                    return;
                }
                else if (vertex_kind::synthetic == g[v].type) { // synthetic events have no pid so there is nothing to do here.
                    logging::debug() << "Synthetic Event ... doing nothing....";
                    return;
                }
                else if (vertex_kind::sync_event == g[v].type) {
                    // we are on a sync event and have no open set
                    logging::debug() << "Sync Event " << v << " ... create a new set";
                    const auto sync_root = root_of_sync(v, g);
                    // create a new set
                    create_new_set(cur_pid, sync_root);
                }
            }
            else { // an open set for this process was found
                logging::debug() << "Open set found!";
                if (vertex_kind::io_event == g[v].type) {
                    logging::debug() << "I/O Event ... ";
                    // open set and on I/O event -> insert I/O event into set
                    set_ptr->insert(v);
                    logging::debug() << "insert vertex #" << v << " @ rank "
                        << g[v].id() << " " << g[v].name() << " into current set";
                }
                else if (vertex_kind::synthetic == g[v].type) { // set should be already closed during examination of the edges!
                    logging::fatal() << "Synthetic Event ... THIS SHOULD NEVER HAPPEN";
                    return;
                }
                else if (vertex_kind::sync_event == g[v].type) {
                    logging::debug() << "Sync Event " << g[v].name()
                        << " ... close current set @" << g[v].id();
                    set_ptr->close();
                    const auto sync_root = root_of_sync(v, g);
                    set_ptr->set_end_event(sync_root);
                    const auto out_dgr = boost::out_degree(v, g);
                    if (out_dgr > 0) { // just create a new set if we are not the last event, here it could be maybe better to have a look at our adjacent vertices
                        logging::debug() << "create a new set for pid: " << cur_pid;
                        create_new_set(cur_pid, sync_root);
                    }
                }
            }
        }

        template<typename Edge, typename Graph>
        void examine_edge(Edge e, const Graph& g)
        {
            const auto src_vd = source(e, g);
            const auto trg_vd = target(e, g);
            logging::debug() << "on examine edge from " << src_vd << " to " << trg_vd;
            const auto src_pid = g[src_vd].id();
            const auto trg_pid = g[trg_vd].id();

            // close current set if synthetic end event is found!
            if (g[trg_vd].type == vertex_kind::synthetic) {
                auto *set_ptr = find_open_set_for(src_pid);
                if (set_ptr == nullptr) {
                    logging::fatal() << "No set was found on the way to synthetic event! This was not expected.";
                    return;
                }
                else {
                    logging::debug() << "on end for pid: " << src_pid << " close set";
                    set_ptr->close();
                    set_ptr->set_end_event(trg_vd);
                }
            }

            // close current set if we reach a sync event next.
            // This is necessary for backtracking during dfs.
            if (src_pid == trg_pid) {
                auto *set_ptr = find_open_set_for(trg_pid);
                if (set_ptr == nullptr) {
                    return;
                }
                if (g[trg_vd].type == vertex_kind::sync_event) {
                    set_ptr->close();
                    const auto sync_root = root_of_sync(trg_vd, g);
                    set_ptr->set_end_event(sync_root);
                }
            }

            // create new set if we come from synthetic root event.
            if (g[src_vd].type == vertex_kind::synthetic) {
                logging::debug() << "source is synthetic ...";
                create_new_set(trg_pid, src_vd);
                if (g[trg_vd].type == vertex_kind::sync_event) {
                    logging::debug() << "target is sync event ... close!";
                    auto *set_ptr = find_open_set_for(trg_pid);
                    if (set_ptr == nullptr) {
                        logging::fatal() << "Error no set found!";
                        return;
                    }
                    set_ptr->close();
                    const auto sync_root = root_of_sync(trg_vd, g);
                    set_ptr->set_end_event(sync_root);
                }
            }
        }

    private:

        template<typename Vertex>
        inline
        void create_new_set(const std::uint64_t pid, Vertex v)
        {
            set_cnt_ptr_->operator[](pid).emplace_back(v);
        }

        //template<typename Set>
        // TODO: use optional instead of trailing return type syntax
        auto find_open_set_for(const std::uint64_t proc_id) -> typename Cont::mapped_type::value_type*
        {
            if (set_cnt_ptr_->empty() ||
                    proc_id == std::numeric_limits<std::uint64_t>::max())
            {
                return nullptr;
            }
            auto it = std::find_if(set_cnt_ptr_->operator[](proc_id).begin(),
                    set_cnt_ptr_->operator[](proc_id).end(),
                    [](typename Cont::mapped_type::value_type& set) {
                        return Set_State::Open == set.state();
                    });
            if (it == set_cnt_ptr_->operator[](proc_id).end()) {
                return nullptr;
            }
            return &(*it);
        }

    private:
        std::shared_ptr<Cont> set_cnt_ptr_;
};

template<typename SetMap>
void remove_empty_sets(SetMap& sets)
{
    std::for_each(sets.begin(), sets.end(),
            [](auto& proc_sets) {
                remove_empty_sets(proc_sets.second);
            });
}

template<typename Set>
void remove_empty_sets(std::vector<Set>& sets)
{
    sets.erase(std::remove_if(
                sets.begin(),
                sets.end(),
                [](const auto& set) {
                    return set.empty();
                }),
            sets.end());
}


// should maybe a variadic template
template<typename Graph>
std::vector<typename Graph::vertex_descriptor>
get_events_by_kind(Graph& graph, const std::vector<vertex_kind>& kinds)
{
    using vertex_descriptor = typename Graph::vertex_descriptor;
    const auto vp = graph.vertices();
    std::vector<vertex_descriptor> events;
    std::copy_if(vp.first, vp.second, std::back_inserter(events),
            [&kinds,&graph](const vertex_descriptor& vd) {
                return std::any_of(kinds.begin(), kinds.end(),
                        [&vd,&graph](const vertex_kind& kind) {
                            return graph[vd].type == kind;
                        });
            });
    return events;
}

/**
 * @breif sort sets per process with ascending start events of the CIO_Set.
 *
 * @param cio_sets: Reference to the SetMap container.
 *
 * When it comes that sets are not sorted, this can happen during backtracking
 * e.g.. 
 * We can sort them using `sort` or try just to `reverse` the vector.
 * Reversing the vector maybe enough if the only reason for sorting
 * is the reverse order of sets through the backtracking during the DFS.
 */
template<typename SetMap>
void sort_sets_by_descriptor(SetMap& cio_sets)
{
    std::for_each(cio_sets.begin(), cio_sets.end(),
            [](auto& kvp_ps)
            {
                bool sorted = std::is_sorted(
                        std::begin(kvp_ps.second),
                        std::end(kvp_ps.second),
                        [](const auto& set_a, const auto& set_b) {
                            return set_a.start_event() < set_b.start_event();
                        });
                if (!sorted) {
                    //try to reverse
                    //std::reverse(kvp_ps.second.begin(), kvp_ps.second.end());
                    //... or sort conventionally
                    std::sort(std::begin(kvp_ps.second),
                            std::end(kvp_ps.second),
                            [](const auto& set_a, const auto& set_b) {
                                return set_a.start_event() < set_b.start_event();
                            });
                }
            });
}

template<typename Graph>
std::vector<typename Graph::vertex_descriptor>
collect_root_sync_events(Graph& graph)
{
    using vertex_descriptor = typename Graph::vertex_descriptor;
    std::vector<vertex_descriptor> result;
    // get all sync and synthetic events in the graph
    const auto sync_events = get_events_by_kind(graph, {vertex_kind::sync_event, vertex_kind::synthetic});
    // store the root-sync-event in result vector
    std::transform(sync_events.begin(), sync_events.end(),
            std::back_inserter(result),
            [&graph](const vertex_descriptor& vd) {
                if (graph[vd].type == vertex_kind::sync_event) {
                    return root_of_sync(vd, *graph.get());
                }
                else {
                    return vd;
                }
            });
    // sort the events if necessary
    if (!std::is_sorted(result.begin(), result.end())) {
        std::sort(result.begin(), result.end());
    }
    // delete everything but hold unique ones
    result.erase(std::unique(result.begin(), result.end()),
                result.end());

    return result;
}

template<typename Graph, typename Vertex>
void sort_events_chronological(Graph& graph, std::vector<Vertex>& events)
{
    auto chrono_cmp = [&graph](const Vertex& vd_a, const Vertex& vd_b)
        {
            const auto t_a = graph[vd_a].timestamp();
            const auto t_b = graph[vd_b].timestamp();
            return t_a < t_b;
        };

    if (!std::is_sorted(events.begin(), events.end(), chrono_cmp)) {
        logging::debug() << "not chronologically sorted, ... sorting";
        std::sort(events.begin(), events.end(), chrono_cmp);
    }
}

template<typename SetMap, typename Vertex>
//TODO: some sort of set_type should be encoded in the SetMap type or provided
//as general type-alias
std::vector<CIO_Set<Vertex>>
merge_sets(SetMap& set_map, const std::vector<Vertex>& sorted_sync_evts)
{
    std::vector<CIO_Set<Vertex>> merged_sets;
    unsigned int count {0};

    while (!std::all_of(set_map.begin(), set_map.end(),
                [](const auto& proc_sets) {
                    return proc_sets.second.empty();
                }))
    {
        std::vector<Vertex> end_evts;
        CIO_Set<Vertex> cur_set;
        std::transform(set_map.begin(), set_map.end(),
                std::back_inserter(end_evts),
                [&cur_set](const auto& proc_sets)
                {
                    if (!proc_sets.second.empty()) {
                        const auto& first_set = proc_sets.second.front();
                        cur_set.merge(first_set);
                        return first_set.end_event().value();
                    }
                });

        std::cout << "End-Events in iteration: " << count << "\n";
        std::copy(end_evts.begin(), end_evts.end(),
                std::ostream_iterator<Vertex>(std::cout, ", "));
        std::cout << "\n";

        assert(set_map.size() == end_evts.size());
        // it points to first occurence in sorted_sync_evts
        auto first = std::find_first_of(sorted_sync_evts.begin(),
                sorted_sync_evts.end(),
                end_evts.begin(), end_evts.end());

        logging::debug() << "iteration: " << count << " first end-event of set: " << *first;
        // the first end event ends the set.
        cur_set.close();
        cur_set.set_end_event(*first);
        merged_sets.push_back(cur_set);

        for (std::size_t i = 0; i < end_evts.size(); ++i)
        {
            if (*first == end_evts[i]) {
                logging::debug() << "delete first set of proc: " << i;
                set_map[i].erase(set_map[i].begin());
            }
        }
        count++;
    }

    return merged_sets;
}

template<typename Graph>
auto collect_concurrent_io_sets(Graph& graph)
{
    using set_container_t = ProcSetMap<
                                ProcSet<
                                    CIO_Set<
                                        typename Graph::vertex_descriptor>>>;

    auto root = find_root(graph);
    assert(graph[root].type == vertex_kind::synthetic);
    auto shared_set_container(std::make_shared<set_container_t>());
    CIO_Visitor<set_container_t> vis(shared_set_container);
    std::vector<boost::default_color_type> color_map(graph.num_vertices());
    boost::depth_first_visit(*graph.get(), root, vis,
            make_iterator_property_map(color_map.begin(), get(boost::vertex_index, *graph.get())));

    return shared_set_container;
}

// all-in-one version of the alorithm above! needs refactoring
template<typename Graph>
auto gather_concurrent_io_sets(Graph& graph)
{
    using set_container_t = ProcSetMap<
                                ProcSet<
                                    CIO_Set<
                                        typename Graph::vertex_descriptor>>>;

    auto root = find_root(graph);
    assert(graph[root].type == vertex_kind::synthetic);
    auto shared_set_container(std::make_shared<set_container_t>());
    CIO_Visitor<set_container_t> vis(shared_set_container);
    std::vector<boost::default_color_type> color_map(graph.num_vertices());
    boost::depth_first_visit(*graph.get(), root, vis,
            make_iterator_property_map(color_map.begin(), get(boost::vertex_index, *graph.get())));
    sort_sets_by_descriptor(*shared_set_container.get());
    auto sync_evts_v = collect_root_sync_events(graph);
    sort_events_chronological(graph, sync_evts_v);
    auto merged_sets = merge_sets(graph, *shared_set_container.get(), sync_evts_v);
    remove_empty_sets(merged_sets);

    return merged_sets;
}

} // namespace rabbitxx

#endif /* RABBITXX_CIO_SET_HPP */

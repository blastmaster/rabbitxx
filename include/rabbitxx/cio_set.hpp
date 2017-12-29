#ifndef RABBITXX_CIO_SET_HPP
#define RABBITXX_CIO_SET_HPP

#include <rabbitxx/log.hpp>

#include <boost/graph/depth_first_search.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <map>
#include <numeric>
#include <unordered_set>
#include <vector>

namespace rabbitxx
{

enum class State
{
    Open,
    Closed
};

std::ostream&
operator<<(std::ostream& os, const State& state)
{
    switch (state)
    {
        case State::Open:
            os << "Open";
            break;
        case State::Closed:
            os << "Closed";
            break;
    }
    return os;
}

/**
 *
 * @tparam VertexDescriptor must be an integral type
 */
template <typename VertexDescriptor>
class CIO_Set
{
public:
    using value_type = VertexDescriptor;
    using set_t = std::set<value_type>;
    using size_type = typename set_t::size_type;
    using iterator = typename set_t::iterator;
    using const_iterator = typename set_t::const_iterator;

    CIO_Set() = default;

    explicit CIO_Set(const value_type& start_event) : start_evt_(start_event)
    {
    }

    // needed for std::unique
    bool operator==(const CIO_Set<value_type>& other)
    {
        return set_ == other.set();
    }

    bool operator!=(const CIO_Set<value_type>& other)
    {
        return set_ != other.set();
    }

    // needed for std::sort
    bool operator<(const CIO_Set<value_type>& other)
    {
        return set_ < other.set();
    }

    void merge(const CIO_Set<value_type>& other_set)
    {
        // choose earliest start_evt_ of all sets merged into the current.
        start_evt_ = other_set.start_event() < start_evt_ ? other_set.start_event() : start_evt_;
        if (other_set.empty())
        {
            logging::debug() << "in merge, other set is empty... skip!";
            return;
        }

        std::copy(other_set.begin(), other_set.end(), std::inserter(set_, set_.begin()));
    }

    State state() const noexcept
    {
        return state_;
    }

    set_t set() const noexcept
    {
        return set_;
    }

    void close() noexcept
    {
        state_ = State::Closed;
    }

    value_type start_event() const noexcept
    {
        return start_evt_;
    }

    boost::optional<value_type> end_event() const noexcept
    {
        return end_evt_;
    }

    value_type origin() const noexcept
    {
        return origin_end_;
    }

    void set_end_event(const value_type& root_end_event)
    {
        end_evt_ = root_end_event;
    }

    void set_end_event(const value_type& root_end_event, const value_type& origin)
    {
        end_evt_ = root_end_event;
        origin_end_ = origin;
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
    State state_ = State::Open;
    boost::optional<value_type> end_evt_ = boost::none;
    value_type origin_end_ = std::numeric_limits<value_type>::max();
    set_t set_{};
};

template <typename DescriptorType>
inline std::ostream&
operator<<(std::ostream& os, const CIO_Set<DescriptorType>& set)
{
    os << "CIO_Set {\n"
       << "\t[State] " << set.state() << "\n"
       << "\t[Start Evt] " << set.start_event() << "\n";
    if (boost::optional<DescriptorType> end_evt = set.end_event())
    {
        os << "\t[End Evt] " << *end_evt << "\n";
        os << "\t[Origin] " << set.origin() << "\n";
    }
    else
    {
        os << "\t[End Evt] "
           << "NONE\n";
    }
    os << "\t[Events] [ ";
    std::copy(set.begin(), set.end(), std::ostream_iterator<DescriptorType>(os, ", "));
    os << " ]\n};\n";
    return os;
}

// define set-type
template <typename VD>
using set_t = CIO_Set<VD>;

// define vector of sets type
template <typename VD>
using set_container_t = std::vector<set_t<VD>>;

// define set-iterator
template <typename VD>
using set_iter_t = typename set_container_t<VD>::iterator;

template <typename C>
using proc_map_t = std::map<std::uint64_t, C>;

// define map type for sets, mapping proc_id -> [ sets ]
template <typename VD>
using set_map_t = proc_map_t<set_container_t<VD>>;

// define map-view-type
template <typename VD>
using map_view_t = proc_map_t<std::pair<set_iter_t<VD>, set_iter_t<VD>>>;

// define process group type as set from uints
using process_group_t = std::set<std::uint64_t>;

/**
 * Process Group Map
 * Mapping vertex descriptors of events to groups (sets) of processes.
 */
template <typename VertexDescriptor>
using pg_map_t = std::map<VertexDescriptor, process_group_t>;

/**
 * @brief Get all in-going synchronization-events of a given vertex.
 *
 * @param v: The vertex descriptor of the synchronization-event.
 * @param g: A Reference to the Graph.
 *
 * @return Retruns a vector containing the vertex descriptors of all in-going
 * synchronization events from a given vertex v.
 */
template <typename Vertex, typename Graph>
std::vector<Vertex>
get_in_going_syncs(Vertex v, Graph& g)
{
    const auto in_edge_r = boost::in_edges(v, g);
    std::vector<Vertex> results;
    for (auto edge_it = in_edge_r.first; edge_it != in_edge_r.second; ++edge_it)
    {
        const auto src_v = source(*edge_it, g);
        if (vertex_kind::sync_event == g[src_v].type)
        {
            results.push_back(src_v);
        }
    }
    return results;
}

/**
 * @brief Find the root-Event of a given Synchronization Event.
 *
 * @tparam Vertex The vertex-descriptor-type
 * @tparam Graph The graph-type
 * @param v the vertex descriptor of a synchronization-event.
 * @param g a reference to the graph.
 *
 * @return The vertex descriptor of the root-event of the synchronization.
 */
template <typename Vertex, typename Graph>
Vertex
root_of_sync(Vertex v, Graph& g)
{
    assert(vertex_kind::sync_event == g[v].type);
    const auto in_dgr = boost::in_degree(v, g);
    if (in_dgr == 1)
    {
        return v;
    }
    const auto sync_evt = boost::get<sync_event_property>(g[v].property);
    // if p2p event get remote proc
    if (sync_evt.comm_kind == sync_event_kind::p2p)
    {
        const auto p2p_evt = boost::get<peer2peer>(sync_evt.op_data);
        const auto r_proc = p2p_evt.remote_process();
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, g);
        for (const auto s : in_syncs)
        {
            const auto id = g[s].id();
            if (id == r_proc)
            { // Test if r_proc is proc_id from root!
                logging::debug() << "process id matches remote proc id!";
                return s;
            }
        }
    } // if collective
    else if (sync_evt.comm_kind == sync_event_kind::collective)
    {
        const auto coll_evt = boost::get<collective>(sync_evt.op_data);
        // check if collective has an explicit root
        if (coll_evt.has_root())
        {
            const auto root_rank = coll_evt.root();
            if (root_rank == g[v].id())
            { // we *are* the root sync event!
                logging::debug() << " v " << v << " root rank " << root_rank;
                return v; // return our own vertex descriptor
            }
        }
        // look at in_edges
        const auto in_syncs = get_in_going_syncs(v, g);
        for (const auto s : in_syncs)
        {
            const auto in_dgr = boost::in_degree(s, g);
            const auto out_dgr = boost::out_degree(s, g);
            if ((out_dgr >= coll_evt.members().size() - 1) && (in_dgr == 1))
            {
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
template <typename Graph>
typename Graph::vertex_descriptor
find_root(Graph& graph)
{
    const auto vertices = graph.vertices();
    const auto root = std::find_if(
        vertices.first, vertices.second, [&graph](const typename Graph::vertex_descriptor& vd) {
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
template <typename Graph>
std::uint64_t
num_procs(Graph& graph) noexcept
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
 * If the sync-event is wether a collective nor a peer2peer event the returned
 * vector is empty.
 */
std::vector<std::uint64_t>
procs_in_sync_involved(const sync_event_property& sevt)
{
    if (sevt.comm_kind == sync_event_kind::collective)
    {
        const auto coll_evt = boost::get<collective>(sevt.op_data);
        return coll_evt.members();
    }
    if (sevt.comm_kind == sync_event_kind::p2p)
    {
        const auto p2p_evt = boost::get<peer2peer>(sevt.op_data);
        return { sevt.proc_id, p2p_evt.remote_process() };
    }
    return {};
}

// template<typename Graph, typename VertexDescriptor>
// std::vector<std::uint64_t>
// procs_in_sync_involved(Graph& graph, const VertexDescriptor& vd)
//{
// const auto type = graph[vd].type;
// switch (type)
//{
// case vertex_kind::sync_event:
// const auto s_evt = boost::get<sync_event_property>(graph[vd].property);
// return procs_in_sync_involved(s_evt);
// case vertex_kind::synthetic:
// const auto np = num_procs(graph);
// std::vector<std::uint64_t> all_procs(np);
// std::iota(all_procs.begin(), all_procs.end(), 0);
// return all_procs;
// default:
// logging::fatal() << "ERROR in procs_in_sync_involved, invalid synhronization
// event!";
//}
//}

/**
 * Return the number of processes involved in a given synchronization routine.
 */
std::uint64_t
num_procs_in_sync_involved(const sync_event_property& sevt)
{
    return procs_in_sync_involved(sevt).size();
}

// TODO: unused!
template <typename Graph, typename Vertex, typename Visitor>
void
traverse_adjacent_vertices(Graph& graph, Vertex v, Visitor& vis)
{
    typename Graph::adjacency_iterator adj_begin, adj_end;
    for (std::tie(adj_begin, adj_end) = boost::adjacent_vertices(v, *graph.get());
         adj_begin != adj_end; ++adj_begin)
    {
        vis(graph, *adj_begin);
        traverse_adjacent_vertices(graph, *adj_begin, vis);
    }
}

/**
* Checks if the start event of a set `of_v` is an adjacent vertex of the current
* event `cur_v`.
*/
template <typename Vertex, typename Graph>
bool
is_adjacent_event_of(const Vertex of_v, const Vertex cur_v, const Graph& g)
{
    const auto adjacent_r = boost::adjacent_vertices(cur_v, g);
    return std::any_of(
        adjacent_r.first, adjacent_r.second, [&of_v](const Vertex vd) { return vd == of_v; });
}

template <typename Cont>
class CIO_Visitor : public boost::default_dfs_visitor
{
public:
    explicit CIO_Visitor(std::shared_ptr<Cont>& sp) : set_cnt_ptr_(sp)
    {
    }

    template <typename Vertex, typename Graph>
    void discover_vertex(Vertex v, const Graph& g)
    {
        const auto cur_pid = g[v].id();
        auto* set_ptr = find_open_set_for(cur_pid);
        if (set_ptr == nullptr)
        { // no open set was found for this process
            logging::debug() << "No open set was found!";
            if (vertex_kind::io_event == g[v].type)
            {
                logging::fatal() << "I/O Event ... THIS SHOULD NEVER HAPPEN";
                return;
            }
            if (vertex_kind::synthetic == g[v].type)
            { // synthetic events have
                // no pid so there is
                // nothing to do here.
                logging::debug() << "Synthetic Event ... doing nothing....";
                return;
            }
            if (vertex_kind::sync_event == g[v].type)
            {
                // we are on a sync event and have no open set
                logging::debug() << "Sync Event " << v << " ... create a new set";
                const auto sync_root = root_of_sync(v, g);
                // create a new set
                create_new_set(cur_pid, sync_root);
            }
        }
        else
        { // an open set for this process was found
            logging::debug() << "Open set found!";
            if (vertex_kind::io_event == g[v].type)
            {
                logging::debug() << "I/O Event ... ";
                // open set and on I/O event -> insert I/O event into set
                set_ptr->insert(v);
                logging::debug() << "insert vertex #" << v << " @ rank " << g[v].id() << " "
                                 << g[v].name() << " into current set";
            }
            else if (vertex_kind::synthetic == g[v].type)
            { // set should be already closed during examination
                // of the edges!
                logging::fatal() << "Synthetic Event ... THIS SHOULD NEVER HAPPEN";
                return;
            }
            else if (vertex_kind::sync_event == g[v].type)
            {
                logging::debug() << "Sync Event " << g[v].name() << " ... close current set @"
                                 << g[v].id();
                set_ptr->close();
                const auto sync_root = root_of_sync(v, g);
                set_ptr->set_end_event(sync_root, v);
                const auto out_dgr = boost::out_degree(v, g);
                if (out_dgr > 0)
                { // just create a new set if we are not the last
                    // event, here it could be maybe
                    // better to have a look at our adjacent vertices
                    logging::debug() << "create a new set for pid: " << cur_pid;
                    create_new_set(cur_pid, sync_root);
                }
            }
        }
    }

    template <typename Edge, typename Graph>
    void examine_edge(Edge e, const Graph& g)
    {
        const auto src_vd = source(e, g);
        const auto trg_vd = target(e, g);
        logging::debug() << "on examine edge from " << src_vd << " to " << trg_vd;
        const auto src_pid = g[src_vd].id();
        const auto trg_pid = g[trg_vd].id();

        // close current set if synthetic end event is found!
        if (g[trg_vd].type == vertex_kind::synthetic)
        {
            auto* set_ptr = find_open_set_for(src_pid);
            if (set_ptr == nullptr)
            {
                logging::fatal() << "No set was found on the way to synthetic event! "
                                    "This was not expected.";
                return;
            }
            logging::debug() << "on end for pid: " << src_pid << " close set";
            set_ptr->close();
            set_ptr->set_end_event(trg_vd, trg_vd);
        }

        // close current set if we reach a sync event next.
        // This is necessary for backtracking during dfs.
        if (src_pid == trg_pid)
        {
            auto* set_ptr = find_open_set_for(trg_pid);
            if (set_ptr == nullptr)
            {
                return;
            }
            if (g[trg_vd].type == vertex_kind::sync_event)
            {
                set_ptr->close();
                const auto sync_root = root_of_sync(trg_vd, g);
                set_ptr->set_end_event(sync_root, trg_vd);
            }
        }

        // create new set if we come from synthetic root event.
        if (g[src_vd].type == vertex_kind::synthetic)
        {
            logging::debug() << "source is synthetic ...";
            create_new_set(trg_pid, src_vd);
            if (g[trg_vd].type == vertex_kind::sync_event)
            {
                logging::debug() << "target is sync event ... close!";
                auto* set_ptr = find_open_set_for(trg_pid);
                if (set_ptr == nullptr)
                {
                    logging::fatal() << "Error no set found!";
                    return;
                }
                set_ptr->close();
                const auto sync_root = root_of_sync(trg_vd, g);
                set_ptr->set_end_event(sync_root, trg_vd);
            }
        }
    }

private:
    template <typename Vertex>
    inline void create_new_set(const std::uint64_t pid, Vertex v)
    {
        set_cnt_ptr_->operator[](pid).emplace_back(v);
    }

    // TODO: use optional instead of trailing return type syntax
    auto find_open_set_for(const std::uint64_t proc_id) -> typename Cont::mapped_type::value_type* // should be set_t<VD>*
    {
        if (set_cnt_ptr_->empty() || proc_id == std::numeric_limits<std::uint64_t>::max())
        {
            return nullptr;
        }
        auto it = std::find_if(set_cnt_ptr_->operator[](proc_id).begin(),
            set_cnt_ptr_->operator[](proc_id).end(),
            [](typename Cont::mapped_type::value_type& set) { return State::Open == set.state(); });
        if (it == set_cnt_ptr_->operator[](proc_id).end())
        {
            return nullptr;
        }
        return &(*it);
    }

private:
    std::shared_ptr<Cont> set_cnt_ptr_;
};

template <typename VertexDescriptor>
void
remove_empty_sets(set_map_t<VertexDescriptor>& sets)
{
    std::for_each(
        sets.begin(), sets.end(), [](auto& proc_sets) { remove_empty_sets(proc_sets.second); });
}

template <typename VertexDescriptor>
void
remove_empty_sets(std::vector<set_t<VertexDescriptor>>& sets)
{
    sets.erase(
        std::remove_if(sets.begin(), sets.end(), [](const auto& set) { return set.empty(); }),
        sets.end());
}

// should maybe a variadic template
template <typename Graph>
std::vector<typename Graph::vertex_descriptor>
get_events_by_kind(Graph& graph, const std::vector<vertex_kind>& kinds)
{
    using vertex_descriptor = typename Graph::vertex_descriptor;
    const auto vp = graph.vertices();
    std::vector<vertex_descriptor> events;
    std::copy_if(vp.first, vp.second, std::back_inserter(events),
        [&kinds, &graph](const vertex_descriptor& vd) {
            return std::any_of(kinds.begin(), kinds.end(), [&vd, &graph](const vertex_kind& kind) {
                return graph[vd].type == kind;

            });
        });
    return events;
}

/**
 * @brief sort sets per process with ascending end events of the CIO_Set.
 *
 * @param cio_sets: Reference to the SetMap container.
 */
template <typename VertexDescriptor>
void
sort_sets_by_end_event(set_map_t<VertexDescriptor>& cio_sets)
{
    std::for_each(cio_sets.begin(), cio_sets.end(), [](auto& kvp_ps) {
        bool sorted = std::is_sorted(std::begin(kvp_ps.second), std::end(kvp_ps.second),
            [](const auto& set_a, const auto& set_b) {
                return set_a.end_event() < set_b.end_event();
            });
        if (!sorted)
        {
            std::sort(std::begin(kvp_ps.second), std::end(kvp_ps.second),
                [](const auto& set_a, const auto& set_b) {
                    return set_a.end_event() < set_b.end_event();
                });
        }
    });
}

/**
 * XXX: The assert will fail if used on final sets! Since there is no origin.
 * This is not a final solution, it should work since events of the same rank
 * can not overhaul themselve.
 * TODO: Nevertheless, sorting the sets using a adjacency relation might be
 * better.
 */
template <typename Graph, typename VertexDescriptor>
void
sort_set_map_chrono(Graph& graph, rabbitxx::set_map_t<VertexDescriptor>& set_map)
{
    auto chrono_cmp = [&graph](const rabbitxx::set_t<VertexDescriptor>& set_a,
        const rabbitxx::set_t<VertexDescriptor>& set_b) {
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

template <typename Graph>
std::vector<typename Graph::vertex_descriptor>
collect_root_sync_events(Graph& graph)
{
    using vertex_descriptor = typename Graph::vertex_descriptor;
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

template <typename Graph, typename Vertex>
void
sort_events_chrono(Graph& graph, std::vector<Vertex>& events)
{
    auto chrono_cmp = [&graph](const Vertex& vd_a, const Vertex& vd_b) {
        const auto t_a = graph[vd_a].timestamp();
        const auto t_b = graph[vd_b].timestamp();
        return t_a < t_b;
    };

    if (!std::is_sorted(events.begin(), events.end(), chrono_cmp))
    {
        logging::debug() << "not chronologically sorted, ... sorting";
        std::sort(events.begin(), events.end(), chrono_cmp);
    }
}

enum class sync_scope
{
    Local,
    Global
};

inline std::ostream&
operator<<(std::ostream& os, const sync_scope& scope)
{
    switch (scope)
    {
        case sync_scope::Local:
            os << "Local";
            break;
        case sync_scope::Global:
            os << "Global";
            break;
        default:
            os << "NONE";
            break;
    }
    return os;
}

template <typename Graph>
sync_scope
classify_sync(Graph& g, const sync_event_property& sevt)
{
    const auto np = num_procs(g);
    const auto inv = num_procs_in_sync_involved(sevt);
    return np == inv ? sync_scope::Global : sync_scope::Local;
}

// what should be returned in the case that `v` is referencing an I/O-Event?
// One option might be to throw an exception.
// Another option might be to introduce a `sync_scope::None` enumeration. On
// which can be checked outside.
template <typename Graph, typename Vertex>
sync_scope
classify_sync(Graph& g, const Vertex& v)
{
    if (g[v].type == vertex_kind::synthetic)
    {
        return sync_scope::Global;
    }
    const auto& sync_evt_p = boost::get<sync_event_property>(g[v].property);
    return classify_sync(g, sync_evt_p);
}

template <typename VertexDescriptor>
map_view_t<VertexDescriptor>
make_mapview(set_map_t<VertexDescriptor>& smap)
{
    using vertex_descriptor = VertexDescriptor;
    map_view_t<vertex_descriptor> m_v;
    for (auto& setmap_kvp : smap)
    {
        auto iter_p = std::make_pair(setmap_kvp.second.begin(), setmap_kvp.second.end());
        m_v.insert(std::make_pair(setmap_kvp.first, iter_p));
    }

    return m_v;
}

template <typename VertexDescriptor>
map_view_t<VertexDescriptor>
update_view(const process_group_t& pg, map_view_t<VertexDescriptor> map_view)
{
    for (const auto& proc : pg)
    {
        const auto& dbg_cached_value = map_view[proc].first->end_event();
        map_view[proc].first = std::next(map_view[proc].first);
        logging::debug() << "process: " << proc << " increment iterator from " << dbg_cached_value
                         << " to " << map_view[proc].first->end_event();
    }

    return map_view;
}

template <typename Graph>
pg_map_t<typename Graph::vertex_descriptor>
make_local_pgmap(Graph& graph)
{
    using vertex_descriptor = typename Graph::vertex_descriptor;
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

template <typename Graph, typename Vertex>
process_group_t
pg_group(Graph& graph, const Vertex& vd)
{
    if (graph[vd].type == vertex_kind::sync_event)
    {
        const auto& evt_property = boost::get<sync_event_property>(graph[vd].property);
        const auto& inv_proc_v = procs_in_sync_involved(evt_property);
        const auto& scope = classify_sync(graph, evt_property);
        if (scope == sync_scope::Local)
        {
            logging::debug() << "Retrun process_group_t of LOCAL sync-event: " << vd;
            return process_group_t(inv_proc_v.begin(), inv_proc_v.end());
        }
        if (scope == sync_scope::Global)
        {
            logging::debug() << "Retrun process_group_t of GLOBAL sync-event: " << vd;
            return process_group_t(inv_proc_v.begin(), inv_proc_v.end());
        }
        logging::fatal() << "undefined sync_scope not handled! Event: " << vd;
        throw - 1; // TODO: proper error handling!
    }
    if (graph[vd].type == vertex_kind::synthetic)
    {
        logging::debug() << "Retrun process_group_t of GLOBAL synthetic-event: " << vd;
        const auto np = num_procs(graph);
        std::vector<std::uint64_t> all_procs(np);
        std::iota(all_procs.begin(), all_procs.end(), 0);
        return process_group_t(all_procs.begin(), all_procs.end());
    }
    // TODO FIXME XXX synthetic event should return all processes to update map
    // properly otherwise we never found an end!

    return {};
}

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
template <typename Graph, typename VertexDescriptor>
std::vector<VertexDescriptor>
do_merge(Graph& graph, const map_view_t<VertexDescriptor>& map_view,
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
    std::cout << "END-EVENTS:\n";
    std::copy(
        end_evts.begin(), end_evts.end(), std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
    std::cout << "\n";

    const auto e_evts = find_end_events_to_update(graph, end_evts);
    if (!e_evts.empty())
    {
        cur_s.close();
        cur_s.set_end_event(e_evts.back());
        logging::debug() << "create new set:\n" << cur_s;
        merged_sets.push_back(cur_s);
    }
    else
    {
        logging::fatal() << "ERROR NO END EVENT FOUND TO UPDATE! THIS SHOULD NOT HAPPEN!";
        throw -1; //TODO
    }

    return e_evts;
}

// number of combinations of unique end-evt-pairs n(n-1)/2
std::size_t
num_unique_pairs(const std::size_t n)
{
    return n * (n - 1) / 2;
}

// O(n^2)
template <typename VertexDescriptor>
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

template <typename Graph, typename VertexDescriptor>
std::vector<VertexDescriptor>
find_end_events_to_update(Graph& graph, std::vector<VertexDescriptor> end_evts)
{
    auto check_update_func = [end_evts](Graph& graph, const std::set<VertexDescriptor>& try_s) {
        for (const VertexDescriptor& vd : try_s)
        {
            bool update = can_update_end_event(graph, end_evts, vd);
            if (update)
            {
                return vd;
            }
        }
    };

    auto check_update_func_single = [end_evts](Graph& graph, const VertexDescriptor& vd) {
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
        // return end_evts.back();
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
        logging::debug() << "JUST ONE LOCAL EVENT SO JUST RETURN!";
        // return end_evts.back();
        // TODO: check if can update
        return end_evts;
    }

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
            logging::debug() << "independent sync pair: (" << sync_p.first << ", " << sync_p.second
                             << ")";
            independent_syncs.insert(sync_p.first);
            independent_syncs.insert(sync_p.second);
        }
        else
        {
            logging::debug() << "overlapping procs from depending sync pair: "
                             << "(" << sync_p.first << ", " << sync_p.second << ")";
            dependent_syncs.insert(sync_p.first);
            dependent_syncs.insert(sync_p.second);
            std::copy(intersection_procs.begin(), intersection_procs.end(),
                std::back_inserter(overlapping_procs));
        }
    }
    logging::debug() << "OVERLAPPING PROCS:";
    std::copy(overlapping_procs.begin(), overlapping_procs.end(),
        std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
    std::cout << "\n";

    VertexDescriptor update_evt;
    // first check dependent events
    if (!dependent_syncs.empty())
    {
        logging::debug() << "choose from dependent syncs";
        update_evt = check_update_func(graph, dependent_syncs);
    }
    // afterwards check for independent events
    else if (!independent_syncs.empty())
    {
        logging::debug() << "choose from independent syncs";
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

template <typename VertexDescriptor>
bool
can_update_end_event(const process_group_t& pgroup, const std::vector<VertexDescriptor>& end_evts,
    const VertexDescriptor& pivot)
{
    return std::all_of(pgroup.begin(), pgroup.end(),
        [&end_evts, &pivot](const std::uint64_t pid) { return end_evts[pid] == pivot; });
}

template <typename Graph, typename VertexDescriptor>
// inline?
bool
can_update_end_event(
    Graph& graph, const std::vector<VertexDescriptor>& end_evts, const VertexDescriptor& pivot)
{
    const auto pgroup = pg_group(graph, pivot);
    return can_update_end_event(pgroup, end_evts, pivot);
}

template <typename Graph, typename VertexDescriptor>
void
process_sets(Graph& graph, map_view_t<VertexDescriptor> map_view,
    std::vector<set_t<VertexDescriptor>>& merged_sets)
{
    using vertex_descriptor = VertexDescriptor;

    bool on_end = std::all_of(map_view.begin(), map_view.end(),
        [](const auto& p_iters) { return p_iters.second.first == p_iters.second.second; });
    if (on_end)
    {
        logging::debug() << "on end return!";
        return;
    }

    const auto end_events = do_merge(graph, map_view, merged_sets);

    for (const auto& end_evt : end_events)
    {
        logging::debug() << "Choose end-event: " << end_evt << " -> recursive update!";
        const auto lpg = pg_group(graph, end_evt);
        process_sets(graph, update_view<vertex_descriptor>(lpg, map_view), merged_sets);
    }
}

namespace detail
{

template <typename Graph, typename VertexDescriptor>
inline set_container_t<VertexDescriptor>
merge_sets_impl(Graph& graph, set_map_t<VertexDescriptor>& set_map)
{
    set_container_t<VertexDescriptor> merged_sets;
    auto map_view = make_mapview(set_map);
    assert(map_view.size() == set_map.size());
    process_sets(graph, map_view, merged_sets);

    return merged_sets;
}

template <typename Graph, typename VertexDescriptor>
inline set_container_t<VertexDescriptor>
merge_sets_impl_old(Graph& graph, set_map_t<VertexDescriptor>& set_map,
    const std::vector<VertexDescriptor>& sorted_sync_evts)
{
    set_container_t<VertexDescriptor> merged_sets;
    unsigned int count{ 0 };

    while (!std::all_of(set_map.begin(), set_map.end(),
        [](const auto& proc_sets) { return proc_sets.second.empty(); }))
    {
        std::vector<VertexDescriptor> end_evts;
        set_t<VertexDescriptor> cur_set;
        // TODO: not sure if this is safe! Because we operating on a sequence but
        // for empty sets ther is no return! Maybe for-loop is better.
        std::transform(set_map.begin(), set_map.end(), std::back_inserter(end_evts),
            [&cur_set](const auto& proc_sets) {
                if (!proc_sets.second.empty())
                {
                    const auto& first_set = proc_sets.second.front();
                    cur_set.merge(first_set);
                    return first_set.end_event().value();
                }
            });

        std::cout << "End-Events in iteration: " << count << "\n";
        std::copy(end_evts.begin(), end_evts.end(),
            std::ostream_iterator<VertexDescriptor>(std::cout, ", "));
        std::cout << "\n";

        assert(set_map.size() == end_evts.size());
        // it points to first occurence in sorted_sync_evts
        auto first = std::find_first_of(
            sorted_sync_evts.begin(), sorted_sync_evts.end(), end_evts.begin(), end_evts.end());

        logging::debug() << "iteration: " << count << " first end-event of set: " << *first;
        // the first end event ends the set.
        cur_set.close();
        cur_set.set_end_event(*first);
        merged_sets.push_back(cur_set);

        for (std::size_t i = 0; i < end_evts.size(); ++i)
        {
            if (*first == end_evts[i])
            {
                logging::debug() << "delete first set of proc: " << i;
                set_map[i].erase(set_map[i].begin());
            }
        }
        count++;
    }

    return merged_sets;
}

} // namespace rabbitxx::detail

template <typename Graph, typename VertexDescriptor>
set_container_t<VertexDescriptor>
merge_sets(Graph& graph, set_map_t<VertexDescriptor>& set_map)
{
    return detail::merge_sets_impl(graph, set_map);
}

/**
 * Here we get the Sets per process - nothing is merged together!
 */
template <typename Graph>
set_map_t<typename Graph::vertex_descriptor>
cio_sets_per_process(Graph& graph)
{
    using map_t = set_map_t<typename Graph::vertex_descriptor>;

    auto root = find_root(graph);
    assert(graph[root].type == vertex_kind::synthetic);
    auto shared_set_container(std::make_shared<map_t>());
    CIO_Visitor<map_t> vis(shared_set_container);
    std::vector<boost::default_color_type> color_map(graph.num_vertices());
    boost::depth_first_visit(*graph.get(), root, vis,
        make_iterator_property_map(color_map.begin(), get(boost::vertex_index, *graph.get())));
    sort_set_map_chrono(graph, *shared_set_container.get());

    return *shared_set_container.get();
}

// all-in-one version - do merging
template <typename Graph>
set_container_t<typename Graph::vertex_descriptor>
find_cio_sets(Graph& graph)
{
    using map_t = set_map_t<typename Graph::vertex_descriptor>;

    map_t sets_per_process = cio_sets_per_process(graph);
    auto merged_sets = merge_sets(graph, sets_per_process);
    logging::debug() << "Resulting Sets:\n" << "raw size: " << merged_sets.size();
    // remove empty sets
    remove_empty_sets(merged_sets);
    logging::debug() << "w/o empty sets: " << merged_sets.size();
    // sort and remove duplicates
    std::sort(merged_sets.begin(), merged_sets.end());
    merged_sets.erase(std::unique(merged_sets.begin(), merged_sets.end()), merged_sets.end());
    logging::debug() << "unique sets: " << merged_sets.size();

    return merged_sets;
}

} // namespace rabbitxx

#endif /* RABBITXX_CIO_SET_HPP */

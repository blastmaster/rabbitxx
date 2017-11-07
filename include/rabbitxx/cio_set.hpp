#ifndef RABBITXX_CIO_SET_HPP
#define RABBITXX_CIO_SET_HPP

#include <rabbitxx/log.hpp>

#include <boost/graph/depth_first_search.hpp>

#include <map>
#include <unordered_set>
#include <vector>
#include <algorithm>

namespace rabbitxx {

template<typename State>
using State_Map = std::map<std::uint64_t, State>;

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

template<typename EvtType, typename ValueType>
class CIO_Set
{
    using value_type = ValueType;
    using set_t = std::unordered_set<value_type>;
    using size_type = typename set_t::size_type;
    using iterator = typename set_t::iterator;
    using const_iterator = typename set_t::const_iterator;

    public:
    CIO_Set(std::uint64_t num_procs)
    {
        for (std::uint64_t i = 0; i < num_procs; ++i) {
            state_map_.insert(std::make_pair(i, Set_State::Open));
        }
    }

    CIO_Set(std::uint64_t num_procs, const EvtType& start_event)
    {
        for (std::uint64_t i = 0; i < num_procs; ++i) {
            state_map_.insert(std::make_pair(i, Set_State::Open));
        }
        start_evt_ = start_event;
    }

    State_Map<Set_State> state_map() const noexcept
    {
        return state_map_;
    }

    Set_State state_for(std::uint64_t proc_id) noexcept
    {
        assert(state_map_.size() > proc_id);
        return state_map_[proc_id];
    }

    EvtType start_event() const noexcept
    {
        return start_evt_;
    }

    EvtType end_event() const noexcept
    {
        return end_evt_;
    }

    void set_end_event(const EvtType& end_event)
    {
        end_evt_ = end_event;
    }

    void close(std::uint64_t proc_id)
    {
        assert(state_map_.size() > proc_id);
        state_map_[proc_id] = Set_State::Closed;
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
    State_Map<Set_State> state_map_;
    EvtType start_evt_;
    EvtType end_evt_;
    set_t set_;
};

template<typename Set>
std::string dump_state_map(const Set& set)
{
    std::stringstream sstr;
    for (const auto& kvp : set.state_map()) {
        sstr << kvp.first << ": [" << kvp.second << "] ";
    }
    return sstr.str();
}

template<typename EvtType, typename ValueType>
inline std::ostream& operator<<(std::ostream& os, const CIO_Set<EvtType, ValueType>& set)
{
    os << "CIO_Set {\n"
        << "[State] " << dump_state_map(set) << "\n"
        << "[Start Evt] " << set.start_event() << "\n"
        << "[End Evt] " << set.end_event() << "\n"
        << "[Events] [ ";
    std::copy(set.begin(), set.end(), 
            std::ostream_iterator<ValueType>(os, ", "));
    os << " ]\n};\n";
    return os;
}


/**
 * Utility function
 * Finds the synthetic root node of a given graph.
 * Root node is the only vertex with a `vertex_kind::synthetic`.
 * Returns the vertex_descriptor of the root node.
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
 * Get the number of processes involved, in a given I/O Graph.
 * Take the synthetic root vertex and count the out_edges.
 */
template<typename Graph>
std::uint64_t num_procs(Graph& graph) noexcept
{
    const auto root = find_root(graph);
    return static_cast<std::uint64_t>(boost::out_degree(root, *graph.get()));
}

/**
 * Return vector containing all the process id's of the processes participating
 * within a given synchronization event.
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

//template<typename Graph, typename Vertex, typename Visitor>
//void traverse_adjacent_vertices(Graph& graph, Vertex v, Visitor& vis)
//{
    //typename Graph::adjacency_iterator adj_begin, adj_end;
    //for (std::tie(adj_begin, adj_end) = boost::adjacent_vertices(v, *graph.get());
         //adj_begin != adj_end;
        //++adj_begin)
    //{
       ////vis(graph, *adj_begin);
       //traverse_adjacent_vertices(graph, *adj_begin, vis);
    //}
//}

template<typename Cont>
class CIO_Visitor : public boost::default_dfs_visitor
{
    public:
        CIO_Visitor(std::shared_ptr<Cont>& sp, std::uint64_t np) : set_cnt_ptr_(sp), num_procs_(np)
        {
        }

        template<typename Vertex, typename Graph>
        void discover_vertex(Vertex v, const Graph& g)
        {
            auto *set_ptr = find_open_set_for(g[v].id());
            if (set_ptr == nullptr) { // no open set was found for this process
                logging::debug() << "No open set was found!";
                if (vertex_kind::io_event == g[v].type) {
                    logging::debug() << "I/O Event ... THIS SHOULD NEVER HAPPEN";
                    return;
                }
                else if (vertex_kind::synthetic == g[v].type) {
                    logging::debug() << "Synthetic Event ... create a new set";
                    // create a new set
                    set_cnt_ptr_->emplace_back(num_procs_, g[v]);
                }
                else if (vertex_kind::sync_event == g[v].type) {
                    // we are on a sync event and have no open set
                    logging::debug() << "Sync Event ... create a new set";
                    // create a new set
                    set_cnt_ptr_->emplace_back(num_procs_, g[v]);
                }
            }
            else { // an open set for this process was found
                logging::debug() << "Open set found!";
                if (vertex_kind::io_event == g[v].type) {
                    logging::debug() << "I/O Event ... ";
                    // open set and on I/O event -> insert I/O event into set
                    set_ptr->insert(v);
                    logging::debug() << "insert vertex #" << v << " into current set";
                }
                else if (vertex_kind::synthetic == g[v].type) {
                    logging::debug() << "Synthetic Event ... THIS SHOULD NEVER HAPPEN";
                    return;
                }
                else if (vertex_kind::sync_event == g[v].type) {
                    logging::debug() << "Sync Event ... close current set @" << g[v].id();
                    // open set and on sync event -> close set
                    set_ptr->close(g[v].id());
                    // if have end event, check if current sync event is successors of the end event!
                    set_ptr->set_end_event(g[v]);
                    logging::debug() << *set_ptr;
                    logging::debug() << "create a new set!";
                    //TODO do not create every time a new set
                    set_cnt_ptr_->emplace_back(num_procs_, g[v]);

                }
            }
        }

        template<typename Edge, typename Graph>
        void examine_edge(Edge e, const Graph& g)
        {
        }

        template<typename Edge, typename Graph>
        void tree_edge(Edge e, const Graph& g)
        {
        }

    private:
        //template<typename Set>
        // TODO: use optional instead of trailing return type syntax
        auto find_open_set_for(const std::uint64_t proc_id) -> typename Cont::value_type*
        {
            auto it = std::find_if(set_cnt_ptr_->begin(), set_cnt_ptr_->end(),
                    [&proc_id](typename Cont::value_type& set) {
                        return Set_State::Open == set.state_for(proc_id);
                    });
            if (it == std::end(*set_cnt_ptr_.get())) {
                return nullptr;
            }
            return &(*it);
        }

    private:
        std::shared_ptr<Cont> set_cnt_ptr_;
        std::uint64_t num_procs_;
};

template<typename Graph>
auto collect_concurrent_io_sets(Graph& graph)
{
    using set_container_t = std::vector<CIO_Set<
                                otf2_trace_event,
                                typename Graph::vertex_descriptor>>;

    auto root = find_root(graph);
    assert(graph[root].type == vertex_kind::synthetic);
    const auto np = num_procs(graph);
    auto shared_set_container(std::make_shared<set_container_t>());
    CIO_Visitor<set_container_t> vis(shared_set_container, np);
    std::vector<boost::default_color_type> color_map(graph.num_vertices());
    boost::depth_first_visit(*graph.get(), root, vis,
            make_iterator_property_map(color_map.begin(), get(boost::vertex_index, *graph.get())));

    return shared_set_container;
}

} // namespace rabbitxx

#endif /* RABBITXX_CIO_SET_HPP */

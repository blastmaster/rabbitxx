#ifndef RABBITXX_CIO_TYPES_HPP
#define RABBITXX_CIO_TYPES_HPP

#include <rabbitxx/log.hpp>
#include <rabbitxx/graph.hpp>

#include <boost/graph/depth_first_search.hpp>
#include <boost/optional.hpp>

#include <set>
#include <vector>
#include <map>

namespace rabbitxx {

enum class State
{
    Open,
    Close
};

inline
std::ostream&
operator<<(std::ostream& os, const State& state)
{
    switch (state)
    {
        case State::Open:
            os << "Open";
            break;
        case State::Close:
            os << "Close";
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
        if (other_set.empty())
        {
            return;
        }
        //TODO: choose smallest start_evt_, this might not be the earliest.
        start_evt_ = other_set.start_event() < start_evt_ ? other_set.start_event() : start_evt_;
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
        state_ = State::Close;
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

    void set_end_event(const value_type& event)
    {
        end_evt_ = event;
    }

    void set_end_event(const value_type& event, const value_type& origin)
    {
        end_evt_ = event;
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
        if (set_ptr == nullptr) // no open set was found for this process
        {
            if (vertex_kind::sync_event == g[v].type)
            {
                // we are on a sync event and have no open set
                //logging::debug() << "Sync Event " << v << " ... create a new set";
                const auto sync_root = root_of_sync(v, g);
                create_new_set(cur_pid, sync_root);
            }
            if (vertex_kind::io_event == g[v].type) // just to check my assumptions
            {
                logging::fatal() << "I/O Event ... THIS SHOULD NEVER HAPPEN";
                return;
            }
            // synthetic events have no pid so there is nothing to do here.
        }
        else // an open set for this process was found
        {
            //logging::debug() << "Open set found!";
            if (vertex_kind::io_event == g[v].type)
            {
                // open set and on I/O event -> insert I/O event into set
                set_ptr->insert(v);
                //logging::debug() << "insert vertex #" << v << " @ rank " << g[v].id() << " "
                                 //<< g[v].name() << " into current set";
            }
            else if (vertex_kind::synthetic == g[v].type)
            { // set should be already closed during examination
                // of the edges!
                //logging::fatal() << "Synthetic Event ... THIS SHOULD NEVER HAPPEN";
                return;
            }
            else if (vertex_kind::sync_event == g[v].type)
            {
                //logging::debug() << "Sync Event " << g[v].name() << " ... close current set @"
                                 //<< g[v].id();
                set_ptr->close();
                const auto sync_root = root_of_sync(v, g);
                set_ptr->set_end_event(sync_root, v);
                const auto out_dgr = boost::out_degree(v, g);
                // FIXME: this should be the case every time, since we have an synthetic end-event.
                // It was introduced in commit: 06c381d2f8c0e887d279f9e86ea04174d90581fe
                // one commit before the synthetic end event was introduced.
                // Since the synthetic end-event exists this should be always
                // true.
                if (out_dgr > 0)
                { // just create a new set if we are not the last
                    // event, here it could be maybe
                    // better to have a look at our adjacent vertices
                    //logging::debug() << "create a new set for pid: " << cur_pid;
                    create_new_set(cur_pid, sync_root);
                }
                else
                {
                    logging::fatal() << "sync event with out_dgr: " << out_dgr << " THIS SHOULD NEVER HAPPEN!";
                }
            }
        }
    }

    template <typename Edge, typename Graph>
    void examine_edge(Edge e, const Graph& g)
    {
        const auto src_vd = source(e, g);
        const auto trg_vd = target(e, g);
        const auto src_pid = g[src_vd].id();
        const auto trg_pid = g[trg_vd].id();

        //logging::debug() << "on examine edge from " << src_vd << " to " << trg_vd;

        // close current set if synthetic end event is found!
        if (g[trg_vd].type == vertex_kind::synthetic)
        {
            auto* set_ptr = find_open_set_for(src_pid);
            if (set_ptr == nullptr)
            {
                logging::fatal() << "no set was found on the way to synthetic event! THIS SHOULD NEVER HAPPEN!";
                return;
            }
            //logging::debug() << "on end for pid: " << src_pid << " close set";
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
            //logging::debug() << "source is synthetic ...";
            create_new_set(trg_pid, src_vd);
            //FIXME is this necessary?
            //here the newly created set is closed if the target is a
            //sync event, Since next to this we call `discover vertex`
            //where an sync event will causing the set to be closed.
            //We should only create a cio_set if the target isn't a sync event.
            //This saves us to produce empty sets.
            if (g[trg_vd].type == vertex_kind::sync_event)
            {
                //logging::debug() << "target is sync event ... close!";
                auto* set_ptr = find_open_set_for(trg_pid);
                if (set_ptr == nullptr)
                {
                    //logging::fatal() << "Error no set found!";
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
        //auto it = std::find_if(set_cnt_ptr_->operator[](proc_id).begin(),
            //set_cnt_ptr_->operator[](proc_id).end(),
            //[](typename Cont::mapped_type::value_type& set) { return State::Open == set.state(); });
        //if (it == set_cnt_ptr_->operator[](proc_id).end())
        //{
            //return nullptr;
        //}
        //assert(!set_cnt_ptr_->operator[](proc_id).empty());
        if (set_cnt_ptr_->operator[](proc_id).empty())
        {
            return nullptr;
        }
        auto it2 = std::prev(set_cnt_ptr_->operator[](proc_id).end());
        if (it2->state() != State::Open)
        {
            return nullptr;
        }
        //assert(it2->state() == State::Open);
        //assert(*it2 == *it);
        //return &(*it);
        return &(*it2);
    }

private:
    std::shared_ptr<Cont> set_cnt_ptr_;
};

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


} // namespace rabbitxx

#endif // RABBITXX_CIO_TYPES_HPP

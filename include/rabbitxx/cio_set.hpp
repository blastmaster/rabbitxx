#ifndef RABBITXX_CIO_SET_HPP
#define RABBITXX_CIO_SET_HPP

#include <map>
#include <unordered_set>

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
        start_evt_(start_event);
    }

    State_Map<Set_State> state_map() const noexcept
    {
        return state_map_;
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

} // namespace rabbitxx

#endif /* RABBITXX_CIO_SET_HPP */

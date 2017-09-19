#ifndef RABBITXX_LOCATION_QUEUE_HPP
#define RABBITXX_LOCATION_QUEUE_HPP

#include <map>
#include <deque>

namespace rabbitxx {

    template<typename QType>
    class location_queue
    {
    public:
        using value_type = QType;
        using key_type = otf2::reference<otf2::definition::location>::ref_type;
        using container = std::map<key_type, std::deque<value_type>>;

        void enqueue(const otf2::definition::location& location, const value_type& value)
        {
            map_[location.ref()].push_back(value);
        }

        void dequeue(const otf2::definition::location& location)
        {
            map_[location.ref()].pop_front();
        }

        value_type& front(const otf2::definition::location& location)
        {
            return map_[location.ref()].front();
        }

        const value_type& front(const otf2::definition::location& location) const
        {
            return map_[location.ref()].front();
        }

        std::size_t size(const otf2::definition::location& location) //const noexcept
        {
            return map_[location.ref()].size();
        }

        bool empty(const otf2::definition::location& location) //const noexcept
        {
            return map_[location.ref()].empty();
        }

        std::size_t count(const otf2::definition::location& location) const
        {
            return map_.count(location.ref());
        }

        std::deque<value_type>& operator[](const key_type& key)
        {
            return map_[key];
        }

        const std::deque<value_type>& operator[](const key_type& key) const
        {
            return map_[key];
        }

        auto begin()
        {
            return map_.begin();
        }

        auto end()
        {
            return map_.end();
        }

        //auto begin(const otf2::definition::location& location)
        //{
            //return map_[location.ref()].begin();
        //}

        //auto end(const otf2::definition::location& location)
        //{
            //return map_[location.ref()].end();
        //}

    private:
        container map_;
    };

} // namespace rabbitxx

#endif // RABBITXX_LOCATION_QUEUE_HPP

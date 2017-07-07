#ifndef __RABBITXX_MAPPING_HPP__
#define __RABBITXX_MAPPING_HPP__

#include <rabbitxx/log.hpp>

#include <otf2xx/definition/location.hpp>

#include <cassert>
#include <map>

namespace rabbitxx {

    /**
     * Terminilogy:
     * In the following the processes (ranks), which  executing this program are
     * denoted as ranks. The processes recorded in the trace file are denoted as
     * locations. Even if locations may have a "slightly"  more complex
     * definition in otf2.
     *
     * Cases:               Mapping:    Description:
     * ranks = locations    1 : 1
     * ranks > locations    1 : 1       #locations ranks are used the other idle
     * ranks < locations    1 : N       special case use mapping strategy
     *
     * Mapping strategy:
     *
     * Round-Robin:
     * location % ranks = rank whose responsible for process the location
     */

    namespace detail
    {
        struct round_robin_mapping
        {
            explicit round_robin_mapping(int nranks, int nlocations) noexcept
                : ranks_(nranks), locations_(nlocations)
            {
            }

            /**
             * Return a mapping for each rank to a location map<location, rank>.
             */
            std::map<int, int> operator()() const
            {
                std::map<int, int> round_robin_map;
                for (int i = 0; i < locations_; ++i)
                {
                    round_robin_map.emplace(i, i % ranks_);
                }
                return round_robin_map;
            }

        private:
            int ranks_;
            int locations_;
        };

    } // namespace detail

    template<typename mapping_strategy>
    class mapping
    {
        public:
            mapping(int ranks, int num_locations) noexcept
                : strategy_(ranks, num_locations), loc_to_rank_(), rank_to_loc_()
            {
            }

            void register_location(const otf2::definition::location& location)
            {
                assert(to_rank(location.ref()) == -1);

                const auto map = strategy_();
                const auto it = map.find(location.ref());
                if (it == map.end()) {
                    logging::fatal() << "Cannot register location #" << location.ref()
                                     << " no mapping found!";
                    return;
                }
                register_mapping(location, it->second);
            }

            int to_rank(const otf2::definition::location& location) const
            {
                return to_rank(location.ref());
            }

            int to_rank(const otf2::reference<otf2::definition::location>::ref_type ref) const
            {
                if (loc_to_rank_.count(ref) <= 0) {
                    logging::fatal() << "There is no rank registered to location #" << ref;
                    return -1;
                }

                return loc_to_rank_.at(ref);
            }

            const otf2::definition::location to_location(int rank) const
            {
                const auto it = rank_to_loc_.find(rank);
                if (it == rank_to_loc_.end()) {
                    logging::fatal() << "There is no location registered to rank: " << rank;
                }
                return it->second;
            }

        private:
            void register_mapping(const otf2::definition::location& location, int rank)
            {
                assert(to_rank(location.ref()) == -1);

                rank_to_loc_.emplace(rank, location);
                loc_to_rank_.emplace(location.ref(), rank);

                logging::debug() << "Mapped location #" << location.ref() << " to rank: " << rank;

                assert(to_rank(location.ref()) == rank);
            }

            mapping_strategy strategy_;
            std::map<otf2::reference<otf2::definition::location>::ref_type, int> loc_to_rank_;
            std::map<int, otf2::definition::location> rank_to_loc_;
    };


} // namespace rabbitxx

#endif // __RABBITXX_MAPPING_HPP__

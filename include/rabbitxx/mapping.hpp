#ifndef RABBITXX_MAPPING_HPP
#define RABBITXX_MAPPING_HPP

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
             * Return a mapping for each location to a rank map<location, rank>.
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

            // without an explicit amount of ranks, we propably running
            // single-threaded.
            mapping(int num_locations) noexcept : mapping(1, num_locations)
            {
            }

            // should we return the rank we assign the location to?
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
                    logging::debug() << "There is no rank registered to location #" << ref;
                    return -1;
                }

                return loc_to_rank_.at(ref);
            }

            // TODO: this will not work if we have ranks assigned to more than one location!
            const otf2::definition::location to_location(int rank) const
            {
                //FIXME: if multimap is used we should use equal_range(rank) to
                //get all locations assigned to this rank maybe test with
                //count() before.
                auto num_locs = rank_to_loc_.count(rank);
                logging::debug() << num_locs << " registered to rank: " << rank;
                const auto it = rank_to_loc_.find(rank);
                if (it == rank_to_loc_.end()) {
                    logging::fatal() << "There is no location registered to rank: " << rank;
                }
                return it->second;
            }

            auto local_locations(int rank) const
            {
                return rank_to_loc_.equal_range(rank);
            }

            template<typename strategy>
            friend void dump_mapping(const mapping<strategy>& map);

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
            std::map<otf2::reference<otf2::definition::location>::ref_type, int> loc_to_rank_; // no problem locations are unique
            //TODO collision if one rank has more than one location to care about
            // multimap maybe a solution
            std::multimap<int, otf2::definition::location> rank_to_loc_; 
    };

    template<typename Strategy>
    void dump_mapping(const mapping<Strategy>& map)
    {
        logging::debug() << "location -> rank";
        for (const auto& kvp : map.loc_to_rank_)
        {
            logging::debug() << kvp.first << " -> " << kvp.second;
        }
        logging::debug() << "rank -> location";
        for (const auto& kvp : map.rank_to_loc_)
        {
            logging::debug() << kvp.first << " -> " << kvp.second;
        }
    }

} // namespace rabbitxx

#endif // RABBITXX_MAPPING_HPP

#ifndef __RABBITXX_TRACE_BASE_HPP__
#define __RABBITXX_TRACE_BASE_HPP__

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/callback.hpp>

#include <boost/mpi.hpp>

namespace rabbitxx { namespace trace {

    class base : public otf2::reader::callback
    {
        public:

            using otf2::reader::callback::event;
            using otf2::reader::callback::definition;

            base() noexcept = default;
            base(boost::mpi::communicator comm) noexcept
            : rank_(comm.rank()), comm_(comm)
            {
            }

            virtual ~base();

        protected:

            boost::mpi::communicator& comm()
            {
                return comm_.value();
            }

            bool is_master() const noexcept
            {
                return 0 == rank_;
            }

        private:
            int rank_ = 0;
            boost::optional<boost::mpi::communicator> comm_;
    };

    inline base::~base()
    {
    }

}} // namespace rabbitxx::trace

#endif // __RABBITXX_TRACE_BASE_HPP__

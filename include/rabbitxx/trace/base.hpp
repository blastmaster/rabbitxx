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

            base(boost::mpi::communicator comm) noexcept
            : comm_(comm)
            {
            }

            virtual ~base();

            // Events

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::enter& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::leave& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_begin& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_complete& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_acquire_lock& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_change_status_flag& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_create_handle& evt)  override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_delete_file& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_destroy_handle& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_duplicate_handle& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_cancelled& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_issued& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_test& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_release_lock& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_seek& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_try_lock& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_collective_begin& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_collective_end& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_ireceive& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_ireceive_request& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_isend& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_isend_complete& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_receive& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_request_cancelled& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_request_test& evt) override {};

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_send& evt) override {};

            virtual void events_done(const otf2::reader::reader& rdr) override {};

            // Definitions

            virtual void definition(const otf2::definition::string& definition) override {};

            virtual void definition(const otf2::definition::location& definition) override {};

            virtual void definition(const otf2::definition::region& definition) override {};

            virtual void definition(const otf2::definition::comm& definition) override {};

            virtual void definition(const otf2::definition::io_paradigm& definition) override {};

            virtual void definition(const otf2::definition::io_file& definition) override {};

            virtual void definition(const otf2::definition::io_handle& definition) override {};

            virtual void definition(const otf2::definition::io_file_property& definition) override {};

            virtual void definition(const otf2::definition::unknown& definition) override {};

            virtual void definitions_done(const otf2::reader::reader& rdr) override {};

        protected:

            boost::mpi::communicator& comm()
            {
                return comm_;
            }

            bool is_master() const noexcept
            {
                return (0 == comm_.rank());
            }

        private:
            boost::mpi::communicator comm_;
    };

    inline base::~base()
    {
    }

}} // namespace rabbitxx::trace

#endif // __RABBITXX_TRACE_BASE_HPP__

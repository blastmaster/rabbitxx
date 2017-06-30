#ifndef __RABBITXX_TRACE_GRAPH_BUILDER_HPP__
#define __RABBITXX_TRACE_GRAPH_BUILDER_HPP__

#include <rabbitxx/trace/base.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/reader.hpp>

#include <rabbitxx/log.hpp>

namespace rabbitxx { namespace trace {

    template<typename Graph>
    class graph_builder : public rabbitxx::trace::base
    {
        typedef rabbitxx::trace::base base;

        public:

            using otf2::reader::callback::event;
            using otf2::reader::callback::definition;

            graph_builder(boost::mpi::communicator& comm)
            : base(comm), graph_()
            {
                logging::debug() << "graph_builder ctor";
            }

            ~graph_builder() = default;

            // Events

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::enter& evt) override
            {
                logging::debug() << "Found enter event to location #" << location.ref() << " @"
                                 << evt.timestamp();

            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::leave& evt) override
            {
                logging::debug() << "Found leave event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_begin& evt) override
            {
                logging::debug() << "Found io_operation_begin event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_complete& evt) override
            {
                logging::debug() << "Found io_operation_complete event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_acquire_lock& evt) override
            {
                logging::debug() << "Found io_acquire_lock event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_change_status_flag& evt) override
            {
                logging::debug() << "Found io_change_status_flag event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_create_handle& evt) override
            {
                logging::debug() << "Found io_create_handle event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_delete_file& evt) override
            {
                logging::debug() << "Found io_delete_file event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_destroy_handle& evt) override
            {
                logging::debug() << "Found io_destroy_handle event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_duplicate_handle& evt) override
            {
                logging::debug() << "Found io_duplicate_handle event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_cancelled& evt) override
            {
                logging::debug() << "Found io_operation_cancelled event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_issued& evt) override
            {
                logging::debug() << "Found io_operation_issued event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_test& evt) override
            {
                logging::debug() << "Found io_operation_test event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_release_lock& evt) override
            {
                logging::debug() << "Found io_release_lock event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_seek& evt) override
            {
                logging::debug() << "Found io_seek event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_try_lock& evt) override
            {
                logging::debug() << "Found io_try_lock event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_collective_begin& evt) override
            {
                logging::debug() << "Found mpi_collective_begin event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_collective_end& evt) override
            {
                logging::debug() << "Found mpi_collective_end event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_ireceive& evt) override
            {
                logging::debug() << "Found mpi_ireceive event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_ireceive_request& evt) override
            {
                logging::debug() << "Found mpi_ireceive_request event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_isend& evt) override
            {
                logging::debug() << "Found mpi_isend event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_isend_complete& evt) override
            {
                logging::debug() << "Found mpi_isend_complete event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_receive& evt) override
            {
                logging::debug() << "Found mpi_receive event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_request_cancelled& evt) override
            {
                logging::debug() << "Found mpi_request_cancelled event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_request_test& evt) override
            {
                logging::debug() << "Found mpi_request_test event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_send& evt) override
            {
                logging::debug() << "Found mpi_send event to location #" << location.ref() << " @"
                                 << evt.timestamp();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::unknown& evt) override
            {
                logging::warn() << "Found unknown event with timestamp " << evt.timestamp()
                                << " at " << location;
            }

            virtual void events_done(const otf2::reader::reader& rdr) override
            {
            }

            // Definitions

            virtual void definition(const otf2::definition::string& definition) override
            {
                //logging::debug() << "Found string defintion";
            }

            virtual void definition(const otf2::definition::location& definition) override
            {
                logging::debug() << "Found location defintion";
            }

            virtual void definition(const otf2::definition::region& definition) override
            {
                //logging::debug() << "Found region defintion";
            }

            virtual void definition(const otf2::definition::comm& definition) override
            {
                logging::debug() << "Found comm defintion";
            }

            virtual void definition(const otf2::definition::io_paradigm& definition) override
            {
                logging::debug() << "Found io_paradigm defintion";
            }

            virtual void definition(const otf2::definition::io_file& definition) override
            {
                logging::debug() << "Found io_file defintion";
            }

            virtual void definition(const otf2::definition::io_handle& definition) override
            {
                logging::debug() << "Found io_handle defintion";
            }

            virtual void definition(const otf2::definition::io_file_property& definition) override
            {
                logging::debug() << "Found io_file_property defintion";
            }

            virtual void definition(const otf2::definition::unknown& definition) override
            {
                logging::warn() << "Found unknown defintion";
            }

            virtual void definitions_done(const otf2::reader::reader& rdr) override
            {
            }

        private:
            Graph graph_;
    };

}} // namespace rabbitxx::trace

#endif // __RABBITXX_TRACE_GRAPH_BUILDER_HPP__

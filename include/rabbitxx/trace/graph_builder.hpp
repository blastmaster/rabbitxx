#ifndef __RABBITXX_TRACE_GRAPH_BUILDER_HPP__
#define __RABBITXX_TRACE_GRAPH_BUILDER_HPP__

#include <rabbitxx/trace/base.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/mapping.hpp>
#include <rabbitxx/log.hpp>

#include <nitro/lang/quaint_ptr.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/reader.hpp>


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


            Graph& graph()
            {
                return graph_;
            }

            // Events

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::enter& evt) override
            {
                logging::trace() << "Found enter event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                logging::debug() << "In event handler, found enter event with region name: "
                    << evt.region().name();

                //graph_.add_vertex({vertex_event::event_type::enter,
                                   //nitro::lang::make_quaint<otf2::event::enter>(evt)});
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::leave& evt) override
            {
                logging::trace() << "Found leave event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex({vertex_event::event_type::leave,
                                   //nitro::lang::make_quaint<otf2::event::leave>(evt)});
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_begin& evt) override
            {
                logging::trace() << "Found io_operation_begin event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_complete& evt) override
            {
                logging::trace() << "Found io_operation_complete event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_acquire_lock& evt) override
            {
                logging::trace() << "Found io_acquire_lock event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_change_status_flag& evt) override
            {
                logging::trace() << "Found io_change_status_flag event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_create_handle& evt) override
            {
                logging::trace() << "Found io_create_handle event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_delete_file& evt) override
            {
                logging::trace() << "Found io_delete_file event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_destroy_handle& evt) override
            {
                logging::trace() << "Found io_destroy_handle event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_duplicate_handle& evt) override
            {
                logging::trace() << "Found io_duplicate_handle event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_cancelled& evt) override
            {
                logging::trace() << "Found io_operation_cancelled event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_issued& evt) override
            {
                logging::trace() << "Found io_operation_issued event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_operation_test& evt) override
            {
                logging::trace() << "Found io_operation_test event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_release_lock& evt) override
            {
                logging::trace() << "Found io_release_lock event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_seek& evt) override
            {
                logging::trace() << "Found io_seek event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::io_try_lock& evt) override
            {
                logging::trace() << "Found io_try_lock event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_collective_begin& evt) override
            {
                logging::trace() << "Found mpi_collective_begin event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_collective_end& evt) override
            {
                logging::trace() << "Found mpi_collective_end event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_ireceive& evt) override
            {
                logging::trace() << "Found mpi_ireceive event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_ireceive_request& evt) override
            {
                logging::trace() << "Found mpi_ireceive_request event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_isend& evt) override
            {
                logging::trace() << "Found mpi_isend event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_isend_complete& evt) override
            {
                logging::trace() << "Found mpi_isend_complete event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_receive& evt) override
            {
                logging::trace() << "Found mpi_receive event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_request_cancelled& evt) override
            {
                logging::trace() << "Found mpi_request_cancelled event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_request_test& evt) override
            {
                logging::trace() << "Found mpi_request_test event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::mpi_send& evt) override
            {
                logging::trace() << "Found mpi_send event to location #" << location.ref() << " @"
                                 << evt.timestamp();

                //graph_.add_vertex();
            }

            virtual void event(const otf2::definition::location& location,
                               const otf2::event::unknown& evt) override
            {
                logging::warn() << "Found unknown event with timestamp " << evt.timestamp()
                                << " at " << location;

                //graph_.add_vertex();
            }

            virtual void events_done(const otf2::reader::reader& rdr) override
            {
            }

            // Definitions

            virtual void definition(const otf2::definition::string& definition) override
            {
                logging::trace() << "Found string defintion";
            }

            virtual void definition(const otf2::definition::location& definition) override
            {
                logging::trace() << "Found location defintion";
            }

            virtual void definition(const otf2::definition::region& definition) override
            {
                logging::trace() << "Found region defintion";
            }

            virtual void definition(const otf2::definition::comm& definition) override
            {
                logging::trace() << "Found comm defintion";
            }

            virtual void definition(const otf2::definition::io_paradigm& definition) override
            {
                logging::trace() << "Found io_paradigm defintion";
            }

            virtual void definition(const otf2::definition::io_file& definition) override
            {
                logging::trace() << "Found io_file defintion";
            }

            virtual void definition(const otf2::definition::io_handle& definition) override
            {
                logging::trace() << "Found io_handle defintion";
            }

            virtual void definition(const otf2::definition::io_file_property& definition) override
            {
                logging::trace() << "Found io_file_property defintion";
            }

            virtual void definition(const otf2::definition::unknown& definition) override
            {
                logging::warn() << "Found unknown defintion";
            }

            virtual void definitions_done(const otf2::reader::reader& rdr) override
            {
                for(auto location : rdr.locations()) {
                    //TODO: do rank mapping!
                    rdr.register_location(location);
                }
            }

        private:
            Graph graph_;
    };

}} // namespace rabbitxx::trace

#endif // __RABBITXX_TRACE_GRAPH_BUILDER_HPP__

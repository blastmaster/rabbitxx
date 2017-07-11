#ifndef __RABBITXX_TRACE_SIMPLE_GRAPH_BUILDER_HPP__
#define __RABBITXX_TRACE_SIMPLE_GRAPH_BUILDER_HPP__

#include <rabbitxx/trace/base.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/mapping.hpp>
#include <rabbitxx/log.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/reader.hpp>

#include <queue>
#include <cassert>

namespace rabbitxx { namespace trace {

    template<typename Graph>
    class simple_graph_builder : public rabbitxx::trace::base
    {
        typedef rabbitxx::trace::base base;

    public:
        using otf2::reader::callback::event;
        using otf2::reader::callback::definition;
        using mapping_type = mapping<detail::round_robin_mapping>;

        simple_graph_builder(boost::mpi::communicator& comm, int num_locations)
        : base(comm), graph_(), io_ops_started_(), mapping_(comm.size(), num_locations),
          edge_points_(), region_name_queue_()
        {
        }


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

            if (mapping_.to_rank(location) != comm().rank()) {
                return;
            }

            // TODO: not sure if just save the name as string is that clever
            region_name_queue_.push(evt.region().name().str());
        }

        virtual void event(const otf2::definition::location& location,
                           const otf2::event::leave& evt) override
        {
            logging::trace() << "Found leave event to location #" << location.ref() << " @"
                                << evt.timestamp();

            if (mapping_.to_rank(location) != comm().rank()) {
                return;
            }

            // delete saved region name if leave event is reached
            region_name_queue_.pop();
        }

        virtual void event(const otf2::definition::location& location,
                           const otf2::event::io_operation_begin& evt) override
        {
            logging::trace() << "Found io_operation_begin event to location #" << location.ref() << " @"
                                << evt.timestamp();

            if (mapping_.to_rank(location) != comm().rank()) {
                return;
            }

            // here we just save the event for later.
            // An I/O operation will be merged into one single vertex if the
            // corresponding end occurs.
            io_ops_started_.push(evt);
        }

        virtual void event(const otf2::definition::location& location,
                           const otf2::event::io_operation_complete& evt) override
        {
            logging::trace() << "Found io_operation_complete event to location #" << location.ref() << " @"
                                << evt.timestamp();

            if (mapping_.to_rank(location) != comm().rank()) {
                return;
            }

            auto& begin_evt = io_ops_started_.front();
            // matching id seems to be always the same, check for equality anyhow.
            assert(evt.matching_id() == begin_evt.matching_id());

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            // get region name on top of our region name queue
            const auto& region_name = region_name_queue_.front();

            //TODO: which timestamp should we use? start? or end?
            auto vt =
                rabbitxx::vertex_io_event_property(location.ref(), name, region_name,
                                                   begin_evt.bytes_request(),
                                                   evt.bytes_request(), 0,
                                                   rabbitxx::io_operation_option_container(
                                                       begin_evt.operation_mode(),
                                                       begin_evt.operation_flag()),
                                                   evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            // now, try to add an edge from the vertex before of this process to this one.
            if (edge_points_.empty()) {
                logging::debug() << "No vertex in edge_points_ queue.";
            }
            else if (edge_points_.size() > 1) {
                logging::warn() << "More than one vertex in the edge_points_ queue.";
            }
            else {
                const auto& from_vertex = edge_points_.front();
                const auto edge_desc = graph_.add_edge(from_vertex, descriptor);
                if (! edge_desc.second) {
                    logging::fatal() << "Error could not add edge .. this should not happen.";
                }
                edge_points_.pop(); // remove old vertex after adding the edge.
            }
            // store descriptor in queue, for adding edges later
            edge_points_.push(descriptor);
            io_ops_started_.pop();
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

            if (mapping_.to_rank(location) != comm().rank()) {
                return;
            }

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            const auto& region_name = region_name_queue_.front();
            const auto vt =
                rabbitxx::vertex_io_event_property(location.ref(), name,
                                                   region_name, 0, 0, 0,
                                                   rabbitxx::io_creation_option_container(
                                                       evt.status_flags(),
                                                       evt.creation_flags(),
                                                       evt.access_mode()),
                                                   evt.timestamp());

            const auto& descriptor = graph_.add_vertex(vt);
            // now, try to add an edge from the vertex before of this process to this one.
            if (edge_points_.empty()) {
                logging::debug() << "No vertex in edge_points_ queue.";
            }
            else if (edge_points_.size() > 1) {
                logging::warn() << "More than one vertex in the edge_points_ queue.";
            }
            else {
                const auto& from_vertex = edge_points_.front();
                const auto edge_desc = graph_.add_edge(from_vertex, descriptor);
                if (! edge_desc.second) {
                    logging::fatal() << "Error could not add edge .. this should not happen.";
                }
                edge_points_.pop(); // remove old vertex after adding the edge.
            }
            // store descriptor in queue, for adding edges later
            edge_points_.push(descriptor);
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

            if (mapping_.to_rank(location) != comm().rank()) {
                return;
            }

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            const auto& region_name = region_name_queue_.front();
            const auto vt =
                vertex_io_event_property(location.ref(), name,
                                        region_name, 0, 0, 0,
                                        io_operation_option_container(),
                                        evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            // now, try to add an edge from the vertex before of this process to this one.
            if (edge_points_.empty()) {
                logging::debug() << "No vertex in edge_points_ queue.";
            }
            else if (edge_points_.size() > 1) {
                logging::warn() << "More than one vertex in the edge_points_ queue.";
            }
            else {
                const auto& from_vertex = edge_points_.front();
                const auto edge_desc = graph_.add_edge(from_vertex, descriptor);
                if (! edge_desc.second) {
                    logging::fatal() << "Error could not add edge .. this should not happen.";
                }
                edge_points_.pop(); // remove old vertex after adding the edge.
            }
            // store descriptor in queue, for adding edges later
            edge_points_.push(descriptor);
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

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            // get region_name in front of our queue
            const auto region_name = region_name_queue_.front();

            // NOTE: Mapping:
            //       request_size = offset_request
            //       response_size = offset_result
            //       offset = offset_result
            auto vt =
                vertex_io_event_property(location.ref(), name, region_name, evt.offset_request(),
                                         evt.offset_result(), evt.offset_result(), 
                                         evt.seek_option(), evt.timestamp());

            const auto& descriptor = graph_.add_vertex(vt);
            // now, try to add an edge from the vertex before of this process to this one.
            if (edge_points_.empty()) {
                logging::debug() << "No vertex in edge_points_ queue.";
            }
            else if (edge_points_.size() > 1) {
                logging::warn() << "More than one vertex in the edge_points_ queue.";
            }
            else {
                const auto& from_vertex = edge_points_.front();
                const auto edge_desc = graph_.add_edge(from_vertex, descriptor);
                if (! edge_desc.second) {
                    logging::fatal() << "Error could not add edge .. this should not happen.";
                }
                edge_points_.pop(); // remove old vertex after adding the edge.
            }
            // store descriptor in queue, for adding edges later
            edge_points_.push(descriptor);
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
                //do rank mapping!
                mapping_.register_location(location);
                rdr.register_location(location);
            }
        }

    private:
        Graph graph_;
        std::queue<otf2::event::io_operation_begin> io_ops_started_;
        mapping_type mapping_;
        std::queue<typename Graph::vertex_descriptor> edge_points_;
        std::queue<std::string> region_name_queue_;
    };

}} // namespace rabbitxx::trace

#endif // __RABBITXX_TRACE_SIMPLE_GRAPH_BUILDER_HPP__

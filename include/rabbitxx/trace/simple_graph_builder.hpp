#ifndef __RABBITXX_TRACE_SIMPLE_GRAPH_BUILDER_HPP__
#define __RABBITXX_TRACE_SIMPLE_GRAPH_BUILDER_HPP__

#include <rabbitxx/trace/base.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/mapping.hpp>
#include <rabbitxx/log.hpp>
#include <rabbitxx/location_queue.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/reader.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <cassert>

//#define FILTER_RANK if (mapping_.to_rank(location) != comm().rank()) { return; }

#define FILTER_RANK if (!is_master()) { return; }

namespace rabbitxx { namespace trace {


    template<typename Graph>
    class simple_graph_builder : public rabbitxx::trace::base
    {
        typedef rabbitxx::trace::base base;

    public:
        using otf2::reader::callback::event;
        using otf2::reader::callback::definition;
        using process_group = boost::graph::distributed::mpi_process_group;
        using mapping_type = mapping<detail::round_robin_mapping>;

        simple_graph_builder(boost::mpi::communicator& comm, int num_locations)
        : base(comm), io_ops_started_(), mpi_coll_started_(), mapping_(comm.size(), num_locations),
          edge_points_(), region_name_queue_(), events_(), handler_(), graph_(comm)
        {
            int trigger_msg_sync_tag = 54;
            graph_.register_trigger(trigger_msg_sync_tag, handler_);
            graph_.register_simple_trigger(55, this, &simple_graph_builder::simple_handler);
        }

        Graph& graph()
        {
            return graph_;
        }

        /**
         * Return the mapping object. This is mainly used for debugging.
         */
        mapping_type& get_mapping()
        {
            return mapping_;
        }

        void simple_handler(int source, int tag, const typename Graph::vertex_descriptor& data, boost::parallel::trigger_receive_context ctxt)
        {
            const auto root_loc = mapping_.to_location(comm().rank());
            logging::debug() << "in simple trigger: receive from source: " << source
                                << " with tag: " << tag << " @ " << root_loc;
            assert(comm().rank() == root_loc.ref());
        }

    private:

        boost::optional<typename Graph::edge_add_t>
        build_edge(const typename Graph::vertex_descriptor& descriptor,
                   const otf2::definition::location& location)
        {
            if (edge_points_.count(location) == 0) {
                logging::debug() << "edge_points_ has no entry location #" << location.ref()
                    << "try access...";
            }

            if (edge_points_.empty(location)) {
                logging::debug() << "No vertex in edge_points_ queue.";
                // We cannot add an edge if only one vertex are given.
                // So push vertex into queue and return.
                edge_points_.enqueue(location, descriptor);
                return boost::none;
            }

            if (edge_points_.size(location) > 1) {
                logging::warn() << "More than one vertex in the edge_points_ queue.";
            }

            const auto& from_vertex = edge_points_.front(location);
            const auto edge_desc = graph_.add_edge(from_vertex, descriptor);
            if (! edge_desc.second) {
                logging::fatal() << "Error could not add edge .. this should not happen.";
            }
            edge_points_.dequeue(location); // remove old vertex after adding the edge.
            // store descriptor in queue, for adding edges later
            edge_points_.enqueue(location, descriptor);

            return edge_desc;
        }

        struct sync_trigger_handler
        {
            using v_descriptor = typename Graph::vertex_descriptor;
            using trigger_recv_context = boost::graph::distributed::trigger_receive_context;

            void operator()(int source, int tag, const v_descriptor& data, trigger_recv_context cxt) const
            {
                logging::debug() << "In trigger, source: " << source << " tag: " << tag;
            }
        };

    public:
        // Events
        virtual void event(const otf2::definition::location& location,
                           const otf2::event::enter& evt) override
        {
            logging::trace() << "Found enter event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // TODO: not sure if just save the name as string is that clever
            region_name_queue_.enqueue(location, evt.region().name().str());
        }

        virtual void event(const otf2::definition::location& location,
                           const otf2::event::leave& evt) override
        {
            logging::trace() << "Found leave event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // delete saved region name if leave event is reached
            region_name_queue_.dequeue(location);
        }

        virtual void event(const otf2::definition::location& location,
                           const otf2::event::io_operation_begin& evt) override
        {
            logging::trace() << "Found io_operation_begin event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // here we just save the event for later.
            // An I/O operation will be merged into one single vertex if the
            // corresponding end occurs.
            io_ops_started_.enqueue(location, evt);
        }

        virtual void event(const otf2::definition::location& location,
                           const otf2::event::io_operation_complete& evt) override
        {
            logging::trace() << "Found io_operation_complete event to location #" << location.ref() << " @"
                                << evt.timestamp();
            FILTER_RANK
            // get corresponding begin_operation
            auto& begin_evt = io_ops_started_.front(location);
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

            const auto& region_name = region_name_queue_.front(location);
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
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
            io_ops_started_.dequeue(location);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_acquire_lock& evt) override
        {
            logging::trace() << "Found io_acquire_lock event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_change_status_flag& evt) override
        {
            logging::trace() << "Found io_change_status_flag event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_create_handle& evt) override
        {
            logging::trace() << "Found io_create_handle event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // check for parent! to avoid duplication
            if (evt.handle().has_parent()) {
                logging::debug() << "handle has a parent! ... discard";
                return;
                // Since we already have the parent handle recorded... discard!
                // FIXME: This is not quiet correct! How handle more
                // sophisitcated handles? like hdf5.
            }

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            const auto& region_name = region_name_queue_.front(location);
            const auto vt =
                rabbitxx::vertex_io_event_property(location.ref(), name,
                                                   region_name, 0, 0, 0,
                                                   rabbitxx::io_creation_option_container(
                                                       evt.status_flags(),
                                                       evt.creation_flags(),
                                                       evt.access_mode()),
                                                   evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_delete_file& evt) override
        {
            logging::trace() << "Found io_delete_file event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // check if we have a file name or a "non-file" handle
            // TODO: here we have no handle, but we can the file directly
            std::string name;
            if (!evt.file().name().str().empty()) {
                name = evt.file().name().str();
            }
            else {
                logging::warn() << "No filename for delete file event!";
            }

            const auto& region_name = region_name_queue_.front(location);
            const auto vt =
                vertex_io_event_property(location.ref(), name, region_name,
                                         0, 0, 0, io_operation_option_container(),
                                         evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_destroy_handle& evt) override
        {
            logging::trace() << "Found io_destroy_handle event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //check for parent! avoid duplication
            if (evt.handle().has_parent()) {
                logging::debug() << "handle has a parent! ... discard!";
                return;
                // Since we already have the parent handle recorded... discard!
                // FIXME: This is not quiet correct! How handle more
                // sophisitcated handles? like hdf5.
            }

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            const auto& region_name = region_name_queue_.front(location);
            const auto vt =
                vertex_io_event_property(location.ref(), name,
                                        region_name, 0, 0, 0,
                                        io_operation_option_container(),
                                        evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_duplicate_handle& evt) override
        {
            logging::trace() << "Found io_duplicate_handle event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.new_handle().name().str().empty() && !evt.new_handle().file().name().str().empty()) {
                name = evt.new_handle().file().name().str();
            }
            else {
                name = evt.new_handle().name().str();
            }

            const auto& region_name = region_name_queue_.front(location);
            const auto vt =
                vertex_io_event_property(location.ref(), name,
                                        region_name, 0, 0, 0,
                                        io_creation_option_container(evt.status_flags()),
                                        evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_operation_cancelled& evt) override
        {
            logging::trace() << "Found io_operation_cancelled event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_operation_issued& evt) override
        {
            logging::trace() << "Found io_operation_issued event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_operation_test& evt) override
        {
            logging::trace() << "Found io_operation_test event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_release_lock& evt) override
        {
            logging::trace() << "Found io_release_lock event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_seek& evt) override
        {
            logging::trace() << "Found io_seek event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            // check if we have a file name or a "non-file" handle
            std::string name;
            if (evt.handle().name().str().empty() && !evt.handle().file().name().str().empty()) {
                name = evt.handle().file().name().str();
            }
            else {
                name = evt.handle().name().str();
            }

            const auto region_name = region_name_queue_.front(location);
            // NOTE: Mapping:
            //       request_size = offset_request
            //       response_size = offset_result
            //       offset = offset_result
            auto vt =
                vertex_io_event_property(location.ref(), name, region_name, evt.offset_request(),
                                         evt.offset_result(), evt.offset_result(),
                                         evt.seek_option(), evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_try_lock& evt) override
        {
            logging::trace() << "Found io_try_lock event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_collective_begin& evt) override
        {
            logging::trace() << "Found mpi_collective_begin event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            mpi_coll_started_.enqueue(location, evt);

            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_collective_end& evt) override
        {
            logging::trace() << "Found mpi_collective_end event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            const auto& begin_evt = mpi_coll_started_.front(location);
            const auto region_name = region_name_queue_.front(location);

            //logging::debug() << "Found SYNC event with region name: " << region_name
                            //<< " at location #" << location << " with root: " << evt.root();

            const auto co = evt.comm();
            //logging::debug() << "comm name: " << co.name();
            //if (co.has_self_group()) {
                //logging::debug() << "self group name: " << co.self_group().name();
                //logging::debug() << "self group type: " << to_string(co.self_group().type());
                //logging::debug() << "self group size: " << co.self_group().size();
            //}
            //try {
                //logging::debug() << "group name: " << co.group().name();
                //logging::debug() << "group type: " << to_string(co.group().type());
                //logging::debug() << "group size: " << co.group().size();
                //for (const auto& m : co.group().members()) {
                    //logging::debug() << "has member: " << m;
                //}
            //}
            //catch (...)
            //{
            //}

            const auto vt = vertex_sync_event_property(location.ref(), region_name, evt.root(),
                                                       evt.comm().group().members(), evt.timestamp());
            const auto& descriptor = graph_.add_vertex(vt);
            build_edge(descriptor, location); //TODO!
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_ireceive& evt) override
        {
            logging::trace() << "Found mpi_ireceive event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_ireceive_request& evt) override
        {
            logging::trace() << "Found mpi_ireceive_request event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_isend& evt) override
        {
            logging::trace() << "Found mpi_isend event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_isend_complete& evt) override
        {
            logging::trace() << "Found mpi_isend_complete event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_receive& evt) override
        {
            logging::trace() << "Found mpi_receive event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_request_cancelled& evt) override
        {
            logging::trace() << "Found mpi_request_cancelled event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_request_test& evt) override
        {
            logging::trace() << "Found mpi_request_test event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_send& evt) override
        {
            logging::trace() << "Found mpi_send event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::unknown& evt) override
        {
            logging::warn() << "Found unknown event with timestamp " << evt.timestamp()
                            << " at " << location;

            FILTER_RANK
            //graph_.add_vertex();
        }

        virtual void events_done(const otf2::reader::reader& rdr) override
        {
            logging::debug() << "synchronizing ...";
            synchronize(graph_.pg());
            auto k_map = get(&vertex_event_type::type, *graph_.get());
            if (is_master()) {
                for (const auto& loc_events : events_)
                {
                    logging::debug() << "loc: " << loc_events.first;
                    std::deque<typename Graph::vertex_descriptor> collectives;
                    std::copy_if(loc_events.second.begin(), loc_events.second.end(),
                            std::back_inserter(collectives),
                            [&k_map, this](const typename Graph::vertex_descriptor& vd) // copy all sync events
                            {
                                const auto kind = get(k_map, vd);
                                return kind == vertex_kind::sync_event;
                            });

                    auto p_map = get(&vertex_event_type::property, *graph_.get());
                    for (const auto& v : collectives)
                    {
                        auto vertex = boost::get<vertex_sync_event_property>(get(p_map, v));
                        if (vertex.proc_id != vertex.root_rank) {
                            continue;
                        }
                        for (const auto m : vertex.members)
                        {
                            if (vertex.proc_id == m) {
                                continue; // skip myself
                            }
                            //find corresponding collective for every
                            //participating location.
                            auto it = std::find_if(events_[m].begin(), events_[m].end(),
                                    [&k_map](const typename Graph::vertex_descriptor& vd)
                                    {
                                        const auto kind = get(k_map, vd);
                                        return kind == vertex_kind::sync_event;
                                    });
                            if (it == events_[m].end()) {
                                logging::fatal() << "cannot find corresponding collective event";
                                return;
                            }
                            graph_.add_edge(v, *it);
                            events_[m].erase(it);
                        }
                    }
                }
            }
            logging::debug() << "synchronizing ...";
            synchronize(graph_.pg());
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
        location_queue<otf2::event::io_operation_begin> io_ops_started_;
        location_queue<otf2::event::mpi_collective_begin> mpi_coll_started_;
        mapping_type mapping_;
        location_queue<typename Graph::vertex_descriptor> edge_points_;
        location_queue<std::string> region_name_queue_;
        location_queue<typename Graph::vertex_descriptor> events_;
        sync_trigger_handler handler_;
        Graph graph_;
    };

}} // namespace rabbitxx::trace

#endif // __RABBITXX_TRACE_SIMPLE_GRAPH_BUILDER_HPP__

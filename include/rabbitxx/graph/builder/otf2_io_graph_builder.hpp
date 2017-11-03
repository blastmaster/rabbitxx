#ifndef RABBITXX_OTF2_IO_GRAPH_BUILDER_HPP
#define RABBITXX_OTF2_IO_GRAPH_BUILDER_HPP

#include <rabbitxx/trace/base.hpp>
#include <rabbitxx/graph/io_graph_simple.hpp>
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

namespace rabbitxx { namespace graph {

    template<typename Graph>
    class simple_graph_builder : public rabbitxx::trace::base
    {
        typedef rabbitxx::trace::base base;

    public:
        using otf2::reader::callback::event;
        using otf2::reader::callback::definition;
        using mapping_type = mapping<rabbitxx::detail::round_robin_mapping>;

        simple_graph_builder(boost::mpi::communicator& comm, int num_locations)
        : base(comm), io_ops_started_(), mpi_coll_started_(), mapping_(comm.size(), num_locations),
          edge_points_(), region_name_queue_(), events_(), graph_(std::make_unique<Graph>(comm)),
          root_(create_synthetic_root())
        {
        }

        auto graph()
        {
            return std::move(graph_);
        }

        /**
         * Return the mapping object. This is mainly used for debugging.
         */
        mapping_type& get_mapping()
        {
            return mapping_;
        }

    private:

        /**
         * Create synthetic root vertex, this method is called once during
         * initiallzation.
         */
        typename Graph::vertex_descriptor
        create_synthetic_root()
        {
            assert(graph_->num_vertices() == 0);
            return graph_->add_vertex(otf2_trace_event(vertex_kind::synthetic));
        }

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
                // add edge from synthetic root-vertex to the first vertex on
                // this location.
                graph_->add_edge(root_, descriptor);
                return boost::none;
            }

            if (edge_points_.size(location) > 1) {
                logging::warn() << "More than one vertex in the edge_points_ queue.";
            }

            const auto& from_vertex = edge_points_.front(location);
            const auto edge_desc = graph_->add_edge(from_vertex, descriptor);
            if (! edge_desc.second) {
                logging::fatal() << "Error could not add edge .. this should not happen.";
            }
            edge_points_.dequeue(location); // remove old vertex after adding the edge.
            // store descriptor in queue, for adding edges later
            edge_points_.enqueue(location, descriptor);

            return edge_desc;
        }

        std::string get_handle_name(const otf2::definition::io_handle& handle) const
        {
            // check if we have a file name or a "non-file" handle
            if (handle.name().str().empty() && !handle.file().name().str().empty()) {
                return handle.file().name().str();
            }
            return handle.name().str();
        }

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
            const auto name = get_handle_name(evt.handle());
            const auto region_name = region_name_queue_.front(location);
            //TODO: which timestamp should we use? start? or end?
            const auto vt = io_event_property(location.ref(), name, region_name,
                                                   begin_evt.bytes_request(),
                                                   evt.bytes_request(), 0,
                                                   io_operation_option_container(
                                                       begin_evt.operation_mode(),
                                                       begin_evt.operation_flag()),
                                                   evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
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

            const auto name = get_handle_name(evt.handle());
            const auto region_name = region_name_queue_.front(location);
            const auto vt = io_event_property(location.ref(), name,
                                            region_name, 0, 0, 0,
                                            rabbitxx::io_creation_option_container(
                                                evt.status_flags(),
                                                evt.creation_flags(),
                                                evt.access_mode()),
                                            evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
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
            // TODO: here we have no handle, but we can the io_file definition directly
            std::string name;
            if (!evt.file().name().str().empty()) {
                name = evt.file().name().str();
            }
            else {
                logging::warn() << "No filename for delete file event!";
            }

            const auto region_name = region_name_queue_.front(location);
            const auto vt = io_event_property(location.ref(), name, region_name,
                                         0, 0, 0, io_operation_option_container(),
                                         evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
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

            const auto name = get_handle_name(evt.handle());
            const auto region_name = region_name_queue_.front(location);
            const auto vt = io_event_property(location.ref(), name,
                                            region_name, 0, 0, 0,
                                            io_operation_option_container(),
                                            evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::io_duplicate_handle& evt) override
        {
            logging::trace() << "Found io_duplicate_handle event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            const auto name = get_handle_name(evt.new_handle());
            const auto region_name = region_name_queue_.front(location);
            const auto vt = io_event_property(location.ref(), name,
                                            region_name, 0, 0, 0,
                                            io_creation_option_container(evt.status_flags()),
                                            evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
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

            const auto name = get_handle_name(evt.handle());
            const auto region_name = region_name_queue_.front(location);
            // NOTE: Mapping:
            //       request_size = offset_request
            //       response_size = offset_result
            //       offset = offset_result
            auto vt = io_event_property(location.ref(), name, region_name, evt.offset_request(),
                                         evt.offset_result(), evt.offset_result(),
                                         evt.seek_option(), evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
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
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_collective_end& evt) override
        {
            logging::trace() << "Found mpi_collective_end event to location #" << location.ref() << " @"
                                << evt.timestamp();
            FILTER_RANK

            //TODO: begin event is unused atm. but we could use the timestamp to claculate time range.
            //const auto& begin_evt = mpi_coll_started_.front(location);

            const auto region_name = region_name_queue_.front(location);
            std::vector<std::uint64_t> members;
            if (evt.comm().has_self_group()) {
                // TODO: skip sync event if no members are involved!
                // A single process sync with itself is no synchronization.
                if (evt.comm().self_group().size() <= 0) {
                    logging::debug() << "[" << region_name << "] has no members, therefore no synchronizations happens... skip!";
                    return;
                }

                members = evt.comm().self_group().members();
            }
            else {
                members = evt.comm().group().members();
            }

            assert(members.size() != 0); // getting sure!

            const auto vt = sync_event_property(location.ref(), region_name,
                                                collective(evt.root(), members),
                                                evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
        }

        virtual void event(const otf2::definition::location& location,
                            const otf2::event::mpi_ireceive& evt) override
        {
            logging::trace() << "Found mpi_ireceive event to location #" << location.ref() << " @"
                                << evt.timestamp();

            FILTER_RANK

            const auto region_name = region_name_queue_.front(location);
            const auto vt = sync_event_property(location.ref(), region_name,
                                                peer2peer(evt.sender(), evt.msg_tag(),
                                                          evt.msg_length(),
                                                          evt.request_id()),
                                                evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
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

            const auto region_name = region_name_queue_.front(location);
            const auto vt = sync_event_property(location.ref(), region_name,
                                                peer2peer(evt.receiver(), evt.msg_tag(),
                                                          evt.msg_length(),
                                                          evt.request_id()),
                                                evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
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

            const auto region_name = region_name_queue_.front(location);
            const auto vt = sync_event_property(location.ref(), region_name,
                                                peer2peer(evt.sender(), evt.msg_tag(),
                                                          evt.msg_length()),
                                                evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
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

            const auto region_name = region_name_queue_.front(location);
            const auto vt = sync_event_property(location.ref(), region_name,
                                                peer2peer(evt.receiver(), evt.msg_tag(),
                                                          evt.msg_length()),
                                                evt.timestamp());
            const auto& descriptor = graph_->add_vertex(vt);
            build_edge(descriptor, location);
            events_.enqueue(location, descriptor);
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
            auto k_map = get(&otf2_trace_event::type, *(graph_->get())); //get property map of vertex kinds {SYNC,IO, SYNTHETIC}
            if (is_master())
            {
                for (const auto& loc_events : events_)
                {
                    logging::debug() << "processing location: " << loc_events.first;
                    std::deque<typename Graph::vertex_descriptor> sync_events;
                    //Copy all synchronization events in separate vector.
                    std::copy_if(loc_events.second.begin(), loc_events.second.end(),
                            std::back_inserter(sync_events),
                            [&k_map](const typename Graph::vertex_descriptor& vd) // copy all sync events
                            {
                                const auto kind = get(k_map, vd);
                                return kind == vertex_kind::sync_event;
                            });
                    // get property map of all properties
                    auto p_map = get(&otf2_trace_event::property, *(graph_->get()));
                    // iterate through all vertex desciptors of sync_events occuring on this location
                    for (const auto& v : sync_events)
                    {
                        auto vertex = boost::get<sync_event_property>(get(p_map, v)); // get the corresponding sync event property
                        // Distinguish between sync_event_kind's atm. just collective and p2p.
                        if (vertex.comm_kind == sync_event_kind::collective)
                        {
                            logging::debug() << "vertex sync_event_kind is collective";
                            auto coll_op = boost::get<collective>(vertex.op_data);
                            ///XXX if vertex.root_rank == OTF2XXX::irgendwas::undefined
                            //          hole alle events aus allen locations und
                            //          nimm als root den kleinsten timestamp
                            if (coll_op.root() <= coll_op.members().size()) { // or just if (coll_op.has_root())
                                // root rank is in the range of members
                                if (vertex.proc_id != coll_op.root()) {
                                    continue; //draw edges only from root!
                                }
                            }
                            for (const auto m : coll_op.members())
                            {
                                if (vertex.proc_id == m) {
                                    continue; // skip myself, do not draw cycles
                                }
                                //find corresponding collective for every participating location.
                                auto it = std::find_if(events_[m].begin(), events_[m].end(),
                                                       [&k_map, &p_map](const typename Graph::vertex_descriptor& vd)
                                                       {
                                                           const auto kind = get(k_map, vd);
                                                           if (kind != vertex_kind::sync_event) {
                                                               return false;
                                                           }
                                                           const auto sync_kind = boost::get<sync_event_property>(get(p_map, vd)).comm_kind;
                                                           return sync_kind == sync_event_kind::collective;
                                                       });
                                if (it == events_[m].end()) {
                                    logging::fatal() << "cannot find corresponding collective event for member: " << m << vertex;
                                    return;
                                }
                                graph_->add_edge(v, *it);
                                events_[m].erase(it);
                            }
                        }
                        else
                        {
                            logging::debug() << "vertex sync_event_kind is p2p";
                            assert(vertex.comm_kind == sync_event_kind::p2p);
                            auto p2p_op = boost::get<peer2peer>(vertex.op_data);
                            const auto remote = p2p_op.remote_process();
                            auto it = std::find_if(events_[remote].begin(), events_[remote].end(),
                                         [&k_map, &p_map, &vertex](const typename Graph::vertex_descriptor& vd)
                                         {
                                            const auto kind = get(k_map, vd);
                                            if (kind != vertex_kind::sync_event) {
                                                return false; //discard if not a sync event
                                            }

                                            const auto sevt = boost::get<sync_event_property>(get(p_map, vd));
                                            if (sevt.comm_kind != sync_event_kind::p2p) {
                                                return false; //discard if not a p2p event
                                            }
                                            const auto p2pevt = boost::get<peer2peer>(sevt.op_data);
                                            if (vertex.proc_id == p2pevt.remote_process()) {
                                                return true;
                                            }
                                            return false;
                                         });
                            if (it == events_[remote].end()) {
                                logging::fatal() << "cannot find corresponding p2p event";
                                return;
                            }
                            graph_->add_edge(v, *it);
                            events_[remote].erase(it);
                        }
                    }
                }
            }
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
        std::unique_ptr<Graph> graph_;
        typename Graph::vertex_descriptor root_;
    };

    struct OTF2_Io_Graph_Builder
    {
        using graph_type = rabbitxx::SimpleIoGraph;

        auto operator()(const std::string trace_file, boost::mpi::communicator& comm) const
        {
            otf2::reader::reader trc_reader(trace_file);
            auto num_locations = trc_reader.num_locations();
            simple_graph_builder<graph_type> builder(comm, num_locations);
            trc_reader.set_callback(builder);
            trc_reader.read_definitions();
            comm.barrier();
            trc_reader.read_events();
            comm.barrier();

            return std::move(builder.graph());
        }
    };

}} // namespace rabbitxx::graph

#endif // RABBITXX_OTF2_IO_GRAPH_BUILDER_HPP

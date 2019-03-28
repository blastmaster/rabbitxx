#include<rabbitxx/graph/builder/otf2_io_graph_builder.hpp>
#include <rabbitxx/log.hpp>

#include <algorithm>
#include <cassert>

//#define FILTER_RANK if (mapping_.to_rank(location) != comm().rank()) { return; }
#define FILTER_RANK if (!is_master()) { return; }

namespace rabbitxx { namespace graph {

/**
* @brief Return the mapping object. This is mainly used for debugging.
*/
io_graph_builder::mapping_type& io_graph_builder::get_mapping()
{
    return mapping_;
}

/**
    * @brief Create synthetic root vertex, this method is called once during initiallzation.
    *
    * @return The vertex descriptor of the new synthetic root-event.
    */
VertexDescriptor
io_graph_builder::create_synthetic_root()
{
    assert(graph_.num_vertices() == 0);
    /**
        * FIXME:
        * The timestamp argument is rather a hack to avoid having larger timestamps for the
        * start event than for succeeding one.
        * One possible solution may to get the correct start time from the clock properties.
        * But since this method is called in the constructors initialisation list.
        **/
    const auto& vt = synthetic_event_property("Root", otf2::chrono::time_point(otf2::chrono::duration(0)));
    return graph_.add_vertex(otf2_trace_event(vt));
}

/**
    * @brief Create synthetic end vertex, this method is called after all events are processed.
    */
void io_graph_builder::create_synthetic_end()
{
    const auto& vt = synthetic_event_property("End", otf2::chrono::time_point::max());
    const auto end_descriptor = graph_.add_vertex(otf2_trace_event(vt));
    //get last event from each location
    for (const auto& loc : locations_)
    {
        const auto last_proc_event = edge_points_.front(loc);
        graph_.add_edge(last_proc_event, end_descriptor);
    }
}

IoGraph::edge_t
io_graph_builder::build_edge(const VertexDescriptor& descriptor,
            const otf2::definition::location& location)
{
    if (edge_points_.empty(location)) {
        // if empty, add edge from synthetic root-vertex to the first vertex on
        // this location.
        edge_points_.enqueue(location, descriptor);
        auto ed = graph_.add_edge(root_, descriptor);
        return ed;
    }
    assert(edge_points_.size(location) <= 1);
    const auto& from_vertex = edge_points_.front(location);
    const auto edge_desc = graph_.add_edge(from_vertex, descriptor);
    if (! edge_desc.second) {
        logging::fatal() << "Error could not add edge .. this should not happen.";
    }
    edge_points_.dequeue(location); // remove old vertex after adding the edge.
    // Store descriptor in queue, for adding edges later.
    edge_points_.enqueue(location, descriptor);

    return edge_desc;
}

void io_graph_builder::set_graph_properties()
{
    assert(total_time_ > total_file_io_time_);
    assert(total_time_ > total_file_io_metadata_time_);
    assert(max_tp_ > min_tp_);
    graph_.get()->operator[](boost::graph_bundle).total_time = total_time_;
    graph_.get()->operator[](boost::graph_bundle).io_time = total_file_io_time_;
    graph_.get()->operator[](boost::graph_bundle).io_metadata_time = total_file_io_metadata_time_;
    graph_.get()->operator[](boost::graph_bundle).first_event_time = min_tp_;
    graph_.get()->operator[](boost::graph_bundle).last_event_time = max_tp_;
    // set clock properties
    graph_.get()->operator[](boost::graph_bundle).clock_props = clock_props_;
    graph_.get()->operator[](boost::graph_bundle).file_to_fs = file_to_fs_map_;
    graph_.get()->operator[](boost::graph_bundle).num_locations = locations_.size();
}

void io_graph_builder::check_time(otf2::chrono::time_point tp)
{
    using std::max;
    using std::min;

    min_tp_ = min(min_tp_, tp);
    max_tp_ = max(max_tp_, tp);
}

//FIXME: get_io_handle_name would be a better name since we actually work on a `io_handle`.
std::string io_graph_builder::get_handle_name(const otf2::definition::io_handle& handle) const
{
    // check if we have a file name or a "non-file" handle
    if (handle.name().str().empty() && !handle.file().name().str().empty()) {
        return handle.file().name().str();
    }
    return handle.name().str();
}

// ==================== Event callbacks ====================

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::enter& evt)
{
    logging::trace() << "Found enter event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    check_time(evt.timestamp());
    call_stack_.enqueue(location, stack_frame(evt.timestamp(), otf2::chrono::duration(0)));
    // TODO: not sure if just save the name as string is that clever
    region_name_queue_.push(location, evt.region().name().str());
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::leave& evt)
{
    logging::trace() << "Found leave event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    check_time(evt.timestamp());
    auto cur_frame = call_stack_.front(location);
    auto duration = evt.timestamp() - cur_frame.enter;
    if (cur_frame.vertex != IoGraph::null_vertex())
    {
        auto& cur_vertex = graph_[cur_frame.vertex];
        //cur_vertex.duration = duration;
        cur_vertex.duration = {duration, cur_frame.enter, evt.timestamp()};
    }
    else {
        logging::trace() << "Invalid vertex descriptor";
    }

    total_time_ += duration;

    if (evt.region().role() == otf2::common::role_type::file_io)
    {
        total_file_io_time_ += duration;
    }
    else if (evt.region().role() == otf2::common::role_type::file_io_metadata)
    {
        total_file_io_metadata_time_ += duration;
    }

    // delete saved region name if leave event is reached
    region_name_queue_.pop(location);
    call_stack_.dequeue(location);
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_operation_begin& evt)
{
    logging::trace() << "Found io_operation_begin event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    // here we just save the event for later.
    // An I/O operation will be merged into one single vertex if the
    // corresponding complete event occurs.
    io_ops_started_.enqueue(location, evt);
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_operation_complete& evt)
{
    logging::trace() << "Found io_operation_complete event to location #" << location.ref() << " @"
                        << evt.timestamp();
    FILTER_RANK
    // get corresponding begin_operation
    auto& begin_evt = io_ops_started_.front(location);
    // matching id seems to be always the same, check for equality anyhow.
    assert(evt.matching_id() == begin_evt.matching_id());
    const auto name = get_handle_name(evt.handle());
    const auto region_name = region_name_queue_.top(location);
    //Check whether this is a read, write or flush event.
    io_event_kind kind = io_event_kind::none;
    switch (begin_evt.operation_mode())
    {
        case otf2::common::io_operation_mode_type::read:
            kind = io_event_kind::read;
            // increment offset for read ops
            offset_map_.get_value(location, name).inc(evt.bytes_request());
            break;
        case otf2::common::io_operation_mode_type::write:
            kind = io_event_kind::write;
            // increment offset for write ops
            offset_map_.get_value(location, name).inc(evt.bytes_request());
            break;
        case otf2::common::io_operation_mode_type::flush:
            kind = io_event_kind::flush;
            break;
    }

    auto duration = evt.timestamp() - begin_evt.timestamp();
    //use end timestamp so that, end_t - duration.count() == start
    const auto vt = io_event_property(location.ref(),
                                    name,
                                    region_name,
                                    evt.handle().paradigm().name().str(),
                                    begin_evt.bytes_request(),
                                    evt.bytes_request(),
                                    offset_map_.get_value(location, name).get(), // offset
                                    //0, /* offset */
                                    io_operation_option_container(
                                        begin_evt.operation_mode(),
                                        begin_evt.operation_flag()),
                                    kind,
                                    duration,
                                    evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    call_stack_.front(location).vertex = descriptor;
    io_ops_started_.dequeue(location);
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_acquire_lock& evt)
{
    logging::trace() << "Found io_acquire_lock event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_change_status_flag& evt)
{
    logging::trace() << "Found io_change_status_flag event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_create_handle& evt)
{
    logging::trace() << "Found io_create_handle event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    // check for parent! to avoid duplication
    if (evt.handle().has_parent()) {
        //logging::debug() << "handle has a parent! ... discard";
        return;
        // Since we already have the parent handle recorded... discard!
        // FIXME: This is not quiet correct! How handle more
        // sophisitcated handles? like hdf5.
    }

    const auto name = get_handle_name(evt.handle());
    // check that there is no existing offset_tracker for this file on this location
    // create new offset tracker

    offset_map_.get_value(location, name) = offset_tracker();

    const auto region_name = region_name_queue_.top(location);
    const auto vt = io_event_property(location.ref(),
                                    name,
                                    region_name,
                                    evt.handle().paradigm().name().str(),
                                    0, /* request size */
                                    0, /* response size */
                                    offset_map_.get_value(location, name).get(), /* offset */
                                    rabbitxx::io_creation_option_container(
                                        evt.status_flags(),
                                        evt.creation_flags(),
                                        evt.access_mode()),
                                    io_event_kind::create,
                                    boost::none,
                                    evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_delete_file& evt)
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

    const auto region_name = region_name_queue_.top(location);
    // delete offset_tracker
    size_t res = offset_map_[location.ref()].erase(name);
    assert(res > 0);
    const auto vt = io_event_property(location.ref(),
                                    name,
                                    region_name,
                                    evt.paradigm().name().str(),
                                    0, /* request size */
                                    0, /* repsonse size */
                                    0, /* offset */
                                    io_operation_option_container(),
                                    io_event_kind::delete_or_close,
                                    boost::none,
                                    evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_destroy_handle& evt)
{
    logging::trace() << "Found io_destroy_handle event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    //check for parent! avoid duplication
    if (evt.handle().has_parent()) {
        //logging::debug() << "handle has a parent! ... discard!";
        return;
        // Since we already have the parent handle recorded... discard!
        // FIXME: This is not quiet correct! How handle more
        // sophisitcated handles? like hdf5.
    }

    const auto name = get_handle_name(evt.handle());
    // delete offset_tracker
    size_t res = offset_map_[location.ref()].erase(name);
    assert(res > 0);
    const auto region_name = region_name_queue_.top(location);
    const auto vt = io_event_property(location.ref(),
                                    name,
                                    region_name,
                                    evt.handle().paradigm().name().str(),
                                    0, /* request size */
                                    0, /* response size */
                                    0, /* offset */
                                    io_operation_option_container(),
                                    io_event_kind::delete_or_close,
                                    boost::none,
                                    evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_duplicate_handle& evt)
{
    logging::trace() << "Found io_duplicate_handle event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    const auto name = get_handle_name(evt.new_handle());
    const auto region_name = region_name_queue_.top(location);
    const auto vt = io_event_property(location.ref(),
                                    name,
                                    region_name,
                                    evt.new_handle().paradigm().name().str(),
                                    0, /* request size */
                                    0, /* response size */
                                    0, /* offset */
                                    io_creation_option_container(evt.status_flags()),
                                    io_event_kind::dup,
                                    boost::none,
                                    evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_operation_cancelled& evt)
{
    logging::trace() << "Found io_operation_cancelled event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_operation_issued& evt)
{
    logging::trace() << "Found io_operation_issued event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_operation_test& evt)
{
    logging::trace() << "Found io_operation_test event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_release_lock& evt)
{
    logging::trace() << "Found io_release_lock event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_seek& evt)
{
    logging::trace() << "Found io_seek event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    const auto name = get_handle_name(evt.handle());
    const auto region_name = region_name_queue_.top(location);
    // NOTE: Mapping:
    //       request_size = offset_request
    //       response_size = offset_result
    //       offset = offset_result

    // set offset to seek result!
    offset_map_.get_value(location, name).set(evt.offset_result());

    auto vt = io_event_property(location.ref(), name, region_name,
                                    evt.handle().paradigm().name().str(),
                                    evt.offset_request(),
                                    evt.offset_result(),
                                    offset_map_.get_value(location, name).get(), //offset
                                    evt.seek_option(),
                                    io_event_kind::seek,
                                    boost::none,
                                    evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::io_try_lock& evt)
{
    logging::trace() << "Found io_try_lock event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_collective_begin& evt)
{
    logging::trace() << "Found mpi_collective_begin event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    // nothing todo here!?!
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_collective_end& evt)
{
    logging::trace() << "Found mpi_collective_end event to location #" << location.ref() << " @"
                        << evt.timestamp();
    FILTER_RANK

    const auto region_name = region_name_queue_.top(location);
    std::vector<std::uint64_t> members;
    if (evt.comm().has_self_group()) {
        // TODO: skip sync event if no members are involved!
        // A single process sync with itself is no synchronization.
        if (evt.comm().self_group().size() <= 0) {
            //logging::debug() << "[" << region_name << "] has no members, therefore no synchronizations happens... skip!";
            return;
        }

        members = evt.comm().self_group().members();
    }
    else {
        members = evt.comm().group().members();
    }

    assert(!members.empty()); // getting sure!

    const auto vt = sync_event_property(location.ref(), region_name,
                                        collective(evt.root(), members),
                                        evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    synchronizations_.enqueue(location, descriptor);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_ireceive& evt)
{
    logging::trace() << "Found mpi_ireceive event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    const auto region_name = region_name_queue_.top(location);
    const auto vt = sync_event_property(location.ref(), region_name,
                                        peer2peer(evt.sender(), evt.msg_tag(),
                                                    evt.msg_length(),
                                                    evt.request_id()),
                                        evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    synchronizations_.enqueue(location, descriptor);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_ireceive_request& evt)
{
    logging::trace() << "Found mpi_ireceive_request event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
    //graph_.add_vertex();
}

//TODO
void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_isend& evt)
{
    logging::trace() << "Found mpi_isend event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    const auto region_name = region_name_queue_.top(location);
    const auto vt = sync_event_property(location.ref(), region_name,
                                        peer2peer(evt.receiver(), evt.msg_tag(),
                                                    evt.msg_length(),
                                                    evt.request_id()),
                                        evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    synchronizations_.enqueue(location, descriptor);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_isend_complete& evt)
{
    logging::trace() << "Found mpi_isend_complete event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
    //graph_.add_vertex();
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_receive& evt)
{
    logging::trace() << "Found mpi_receive event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    const auto region_name = region_name_queue_.top(location);
    const auto vt = sync_event_property(location.ref(), region_name,
                                        peer2peer(evt.sender(), evt.msg_tag(),
                                                    evt.msg_length()),
                                        evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    synchronizations_.enqueue(location, descriptor);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_request_cancelled& evt)
{
    logging::trace() << "Found mpi_request_cancelled event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_request_test& evt)
{
    logging::trace() << "Found mpi_request_test event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::mpi_send& evt)
{
    logging::trace() << "Found mpi_send event to location #" << location.ref() << " @"
                        << evt.timestamp();

    FILTER_RANK

    const auto region_name = region_name_queue_.top(location);
    const auto vt = sync_event_property(location.ref(), region_name,
                                        peer2peer(evt.receiver(), evt.msg_tag(),
                                                    evt.msg_length()),
                                        evt.timestamp());
    const auto& descriptor = graph_.add_vertex(otf2_trace_event(vt));
    build_edge(descriptor, location);
    synchronizations_.enqueue(location, descriptor);
    call_stack_.front(location).vertex = descriptor;
}

void io_graph_builder::event(const otf2::definition::location& location,
                    const otf2::event::unknown& evt)
{
    logging::warn() << "Found unknown event with timestamp " << evt.timestamp()
                    << " at " << location;

    FILTER_RANK
    //graph_.add_vertex();
}

//TODO: merge sync events together but conserve the data from all
//synchronization events.
void io_graph_builder::events_done(const otf2::reader::reader& rdr)
{
    if (is_master())
    {
        create_synthetic_end();
        for (const auto& loc_events : synchronizations_)
        {
            // get property map of all properties
            auto p_map = get(&otf2_trace_event::property, *graph_.get());
            // iterate through all vertex desciptors of sync_events occuring on this location
            for (const auto& v : loc_events.second)
            {
                auto& vertex = boost::get<sync_event_property>(get(p_map, v)); // get the corresponding sync event property
                // Distinguish between sync_event_kind's atm. just collective and p2p.
                if (vertex.comm_kind == sync_event_kind::collective)
                {
                    //logging::debug() << "vertex sync_event_kind is collective";
                    auto coll_op = boost::get<collective>(vertex.op_data);
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
                        auto it = std::find_if(synchronizations_[m].begin(), synchronizations_[m].end(),
                                                [&p_map](const VertexDescriptor& vd)
                                                {
                                                    const auto sync_kind = boost::get<sync_event_property>(get(p_map, vd)).comm_kind;
                                                    return sync_kind == sync_event_kind::collective;
                                                });
                        if (it == synchronizations_[m].end()) {
                            logging::fatal() << "cannot find corresponding collective event for member: "
                                << m << "\n" << vertex;
                            return;
                        }
                        //TODO: ist gefundene collective auch member der aktuellen
                        auto& trg_vertex = boost::get<sync_event_property>(get(p_map, *it));
                        // set root event
                        trg_vertex.root_event = v;
                        graph_.add_edge(v, *it);
                        synchronizations_[m].erase(it);
                    }
                }
                else // peer2peer synchronization event
                {
                    //logging::debug() << "vertex sync_event_kind is p2p";
                    assert(vertex.comm_kind == sync_event_kind::p2p);
                    auto p2p_op = boost::get<peer2peer>(vertex.op_data);
                    const auto remote = p2p_op.remote_process();
                    auto it = std::find_if(synchronizations_[remote].begin(), synchronizations_[remote].end(),
                                    [&p_map, &vertex](const VertexDescriptor& vd)
                                    {
                                        const auto sevt = boost::get<sync_event_property>(get(p_map, vd));
                                        if (sevt.comm_kind != sync_event_kind::p2p) {
                                            return false; //discard if not a p2p event
                                        }
                                        const auto p2pevt = boost::get<peer2peer>(sevt.op_data);
                                        if (vertex.proc_id == p2pevt.remote_process()) {
                                            //logging::debug() << "proc id: " << vertex.proc_id << " remote proc: " << p2pevt.remote_process();
                                            //logging::debug() << "Selected p2p events: "  << vertex  << " -> "<<  sevt;
                                            return true;
                                        }
                                        return false;
                                    });
                    if (it == synchronizations_[remote].end()) {
                        logging::fatal() << "cannot find corresponding p2p event";
                        return;
                    }
                    auto& trg_vertex = boost::get<sync_event_property>(get(p_map, *it));
                    //set root event
                    trg_vertex.root_event = v;
                    graph_.add_edge(v, *it);
                    synchronizations_[remote].erase(it);
                }
                // set sync event as root
                vertex.root_event = v;
            }
        }
        // setting graph properties
        set_graph_properties();
    }
}

// Definitions

void io_graph_builder::definition(const otf2::definition::location& definition)
{
    logging::trace() << "Found location defintion";
    locations_.push_back(definition);
}

void io_graph_builder::definition(const otf2::definition::region& definition)
{
    logging::trace() << "Found region defintion";
}

void io_graph_builder::definition(const otf2::definition::comm& definition)
{
    logging::trace() << "Found comm defintion";
}

void io_graph_builder::definition(const otf2::definition::io_paradigm& definition)
{
    logging::trace() << "Found io_paradigm defintion";
}

void io_graph_builder::definition(const otf2::definition::io_handle& definition)
{
    logging::trace() << "Found io_handle defintion";
}

void io_graph_builder::definition(const otf2::definition::io_file_property& definition)
{
    logging::trace() << "Found io_file_property defintion";
}

void io_graph_builder::definition(const otf2::definition::clock_properties& definition)
{
    logging::trace() << "Found clock_properties definition";
    clock_props_ = definition;
}

void io_graph_builder::definition(const otf2::definition::unknown& definition)
{
    logging::warn() << "Found unknown defintion";
}

void io_graph_builder::definitions_done(const otf2::reader::reader& rdr)
{
    for(const auto& location : rdr.locations()) {
        //do rank mapping!
        mapping_.register_location(location);
        rdr.register_location(location);
    }

    const auto& str_refs = rdr.strings();
    for (const auto& fp : rdr.io_file_properties())
    {
        if (fp.name().str() == "File system")
        {
            auto fs_str = str_refs[fp.value().stringRef];
            if (fs_str.str() == "proc" || fs_str.str() == "sysfs")
            {
                continue;
            }
            file_to_fs_map_[fp.def().name().str()] = fs_str.str();
        }
    }
}

}} // namespace rabbitxx::graph

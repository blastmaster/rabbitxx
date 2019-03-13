#ifndef RABBITXX_OTF2_IO_GRAPH_BUILDER_HPP
#define RABBITXX_OTF2_IO_GRAPH_BUILDER_HPP

#include <rabbitxx/trace/base.hpp>
#include <rabbitxx/graph/io_graph.hpp>
#include <rabbitxx/mapping.hpp>
#include <rabbitxx/location_queue.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/reader.hpp>

#include <boost/optional.hpp>

namespace rabbitxx { namespace graph {

struct stack_frame
{
    stack_frame() = default;
    explicit stack_frame(otf2::chrono::time_point tp, otf2::chrono::duration dur)
        : enter(tp), duration(dur)
    {}

    otf2::chrono::time_point enter = otf2::chrono::armageddon();
    otf2::chrono::duration duration = otf2::chrono::duration(0);
    VertexDescriptor vertex = IoGraph::null_vertex();
};

class io_graph_builder : public rabbitxx::trace::base
{
    typedef rabbitxx::trace::base base;

public:
    using otf2::reader::callback::event;
    using otf2::reader::callback::definition;
    using mapping_type = mapping<rabbitxx::detail::round_robin_mapping>;

    explicit io_graph_builder(boost::mpi::communicator& comm, int num_locations)
    : base(comm), io_ops_started_(), mapping_(comm.size(), num_locations),
        edge_points_(), region_name_queue_(), synchronizations_(), graph_(),
        root_(create_synthetic_root())
    {
    }

    explicit io_graph_builder(int num_locations)
    : base(), io_ops_started_(), mapping_(num_locations),
        edge_points_(), region_name_queue_(), synchronizations_(), graph_(),
        root_(create_synthetic_root())
    {
    }

    auto graph()
    {
        return std::move(graph_);
    }

    /**
        * @brief Return the mapping object. This is mainly used for debugging.
        */
    mapping_type& get_mapping();

private:

    /**
        * @brief Create synthetic root vertex, this method is called once during initiallzation.
        *
        * @return The vertex descriptor of the new synthetic root-event.
        */
    VertexDescriptor create_synthetic_root();

    /**
        * @brief Create synthetic end vertex, this method is called after all events are processed.
        */
    void create_synthetic_end();

    IoGraph::edge_t
    build_edge(const VertexDescriptor& descriptor,
                const otf2::definition::location& location);

    void set_graph_properties();

    void check_time(otf2::chrono::time_point tp);

    //FIXME: get_io_handle_name would be a better name since we actually work on a `io_handle`.
    std::string get_handle_name(const otf2::definition::io_handle& handle) const;

public:
    // Event callbacks

    void event(const otf2::definition::location& location,
                        const otf2::event::enter& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::leave& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_operation_begin& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_operation_complete& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_acquire_lock& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_change_status_flag& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_create_handle& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_delete_file& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_destroy_handle& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_duplicate_handle& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_operation_cancelled& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_operation_issued& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_operation_test& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_release_lock& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_seek& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::io_try_lock& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_collective_begin& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_collective_end& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_ireceive& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_ireceive_request& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_isend& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_isend_complete& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_receive& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_request_cancelled& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_request_test& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::mpi_send& evt) override;

    void event(const otf2::definition::location& location,
                        const otf2::event::unknown& evt) override;

    //TODO: merge sync events together but conserve the data from all
    //synchronization events.
    void events_done(const otf2::reader::reader& rdr) override;

    // Definitions

    void definition(const otf2::definition::location& definition) override;
    void definition(const otf2::definition::region& definition) override;
    void definition(const otf2::definition::comm& definition) override;
    void definition(const otf2::definition::io_paradigm& definition) override;
    void definition(const otf2::definition::io_handle& definition) override;
    void definition(const otf2::definition::io_file_property& definition) override;
    void definition(const otf2::definition::clock_properties& definition) override;
    void definition(const otf2::definition::unknown& definition) override;
    void definitions_done(const otf2::reader::reader& rdr) override;

private:
    location_queue<otf2::event::io_operation_begin> io_ops_started_;
    mapping_type mapping_;
    location_queue<VertexDescriptor> edge_points_;
    location_stack<std::string> region_name_queue_;
    location_queue<VertexDescriptor> synchronizations_;
    IoGraph graph_;
    VertexDescriptor root_;
    std::vector<otf2::definition::location> locations_;
    location_queue<stack_frame> call_stack_;
    otf2::chrono::duration total_time_ = otf2::chrono::duration(0);
    otf2::chrono::duration total_file_io_time_ = otf2::chrono::duration(0);
    otf2::chrono::duration total_file_io_metadata_time_ = otf2::chrono::duration(0);
    otf2::chrono::time_point min_tp_ = otf2::chrono::genesis();
    otf2::chrono::time_point max_tp_ = otf2::chrono::armageddon();
    otf2::definition::clock_properties clock_props_;
    std::map<std::string, std::string> file_to_fs_map_ {};
};

struct OTF2_Io_Graph_Builder
{
    using graph_type = rabbitxx::IoGraph;

    //TODO: should be better use decltype(auto) or just return graph_type.
    auto operator()(const std::string& trace_file, boost::mpi::communicator& comm) const
    {
        otf2::reader::reader trc_reader(trace_file);
        auto num_locations = trc_reader.num_locations();
        io_graph_builder builder(comm, num_locations);
        trc_reader.set_callback(builder);
        trc_reader.read_definitions();
        comm.barrier();
        trc_reader.read_events();
        comm.barrier();

        return builder.graph();
    }

    // non-mpi version, overload without communicatior
    auto operator()(const std::string& trace_file) const
    {
        otf2::reader::reader trc_reader(trace_file);
        auto num_locations = trc_reader.num_locations();
        io_graph_builder builder(num_locations);
        trc_reader.set_callback(builder);
        trc_reader.read_definitions();
        trc_reader.read_events();

        return builder.graph();
    }

};

}} // namespace rabbitxx::graph

#endif // RABBITXX_OTF2_IO_GRAPH_BUILDER_HPP

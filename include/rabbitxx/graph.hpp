#ifndef __RABBITXX_GRAPH_HPP__
#define __RABBITXX_GRAPH_HPP__

#include <rabbitxx/log.hpp>

#include <nitro/lang/quaint_ptr.hpp>

#include <otf2xx/event/events.hpp>
#include <otf2xx/chrono/chrono.hpp>

#include <boost/graph/use_mpi.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/distributed/graphviz.hpp>

#include <string>
#include <memory>

namespace rabbitxx {

    //template<typename T>
    //class TD;

    struct vertex_event
    {
        enum event_type
        {
            enter,
            leave,
        } type;

        nitro::lang::quaint_ptr event;

        vertex_event() = default;

        vertex_event(const event_type& evt_type, nitro::lang::quaint_ptr& evt) : type(evt_type), event(std::move(evt))
        {
        }

        vertex_event(const event_type& evt_type, nitro::lang::quaint_ptr&& evt) : type(evt_type), event(std::move(evt))
        {
        }

        vertex_event(const vertex_event& other) : type(other.type)
        {
            auto evt(other.event.as<otf2::event::enter>()); /// copy event //TODO
            //TD<decltype(evt)> evtttt;
            event = nitro::lang::make_quaint<decltype(evt)>(evt); // here we have otf2::event::enter because we use as() above!
            assert(event.get() != nullptr);
        }

        vertex_event& operator=(const vertex_event& other)
        {
            this->type = other.type;
            auto evt(other.event.get()); // with get we get void*
            this->event = nitro::lang::make_quaint<decltype(evt)>(evt); // FIXME: decltype(evt) == void*
            return *this;
        }

        vertex_event(vertex_event&&) = default;
        vertex_event& operator=(vertex_event&&) = default;

        template<typename Archiver>
        void serialize(Archiver& ar, const unsigned int /* version */)
        {
            // TODO: Serialization is mandatory, do something useful here!
        }

    };

// ------------------------------------------------------------------------------------------------

    enum class vertex_kind
    {
        io_event,
        sync_event,
    };

    // helper function, return string representation for io_operation_mode
    std::string to_string(const otf2::common::io_operation_mode_type& mode) noexcept
    {
        switch (mode)
        {
            case otf2::common::io_operation_mode_type::read:
                return {"read"};
            case otf2::common::io_operation_mode_type::write:
                return {"write"};
            case otf2::common::io_operation_mode_type::flush:
                return {"flush"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::io_operation_flag_type& flag) noexcept
    {
        switch (flag)
        {
            case otf2::common::io_operation_flag_type::none:
                return {"none"};
            case otf2::common::io_operation_flag_type::non_blocking:
                return {"non blocking"};
            case otf2::common::io_operation_flag_type::collective:
                return {"collective"};
            default:
                return {"UNDEFINED"};
        }
    }

    // helper function, return string representation for io_seek_option_type
    std::string to_string(const otf2::common::io_seek_option_type& op_type) noexcept
    {
        switch (op_type)
        {
            case otf2::common::io_seek_option_type::from_start:
                return {"start"};
            case otf2::common::io_seek_option_type::from_current:
                return {"current"};
            case otf2::common::io_seek_option_type::from_end:
                return {"end"};
            case otf2::common::io_seek_option_type::data:
                return {"data"};
            case otf2::common::io_seek_option_type::hole:
                return {"hole"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::io_access_mode_type& acc_mode) noexcept
    {
        switch (acc_mode)
        {
            case otf2::common::io_access_mode_type::read_only:
                return {"read only"};
            case otf2::common::io_access_mode_type::write_only:
                return {"write only"};
            case otf2::common::io_access_mode_type::read_write:
                return {"read/write"};
            case otf2::common::io_access_mode_type::execute_only:
                return {"execute only"};
            case otf2::common::io_access_mode_type::search_only:
                return {"search only"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::io_creation_flag_type& creation_flag) noexcept
    {
        switch (creation_flag)
        {
            case otf2::common::io_creation_flag_type::none:
                return {"none"};
            case otf2::common::io_creation_flag_type::create:
                return {"create"};
            case otf2::common::io_creation_flag_type::truncate:
                return {"truncate"};
            case otf2::common::io_creation_flag_type::directory:
                return {"directory"};
            case otf2::common::io_creation_flag_type::exclusive:
                return {"exclusive"};
            case otf2::common::io_creation_flag_type::no_controlling_terminal:
                return {"no controlling terminal"};
            case otf2::common::io_creation_flag_type::no_follow:
                return {"no follow"};
            case otf2::common::io_creation_flag_type::path:
                return {"path"};
            case otf2::common::io_creation_flag_type::temporary_file:
                return {"temporary file"};
            case otf2::common::io_creation_flag_type::largefile:
                return {"large file"};
            case otf2::common::io_creation_flag_type::no_seek:
                return {"no seek"};
            case otf2::common::io_creation_flag_type::unique:
                return {"unique"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::io_status_flag_type& status_flag) noexcept
    {
        switch (status_flag)
        {
            case otf2::common::io_status_flag_type::none:
                return {"none"};
            case otf2::common::io_status_flag_type::close_on_exec:
                return {"close on exec"};
            case otf2::common::io_status_flag_type::append:
                return {"append"};
            case otf2::common::io_status_flag_type::non_blocking:
                return {"non blocking"};
            case otf2::common::io_status_flag_type::async:
                return {"async"};
            case otf2::common::io_status_flag_type::sync:
                return {"sync"};
            case otf2::common::io_status_flag_type::data_sync:
                return {"data sync"};
            case otf2::common::io_status_flag_type::avoid_caching:
                return {"avoid caching"};
            case otf2::common::io_status_flag_type::no_access_time:
                return {"no access time"};
            case otf2::common::io_status_flag_type::delete_on_close:
                return {"delete on close"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::collective_type& coll_type) noexcept
    {
        switch (coll_type)
        {
            case otf2::common::collective_type::barrier:
                return {"barrier"};
            case otf2::common::collective_type::broadcast:
                return {"broadcast"};
            case otf2::common::collective_type::gather:
                return {"gather"};
            case otf2::common::collective_type::gatherv:
                return {"gatherv"};
            case otf2::common::collective_type::scatter:
                return {"scatter"};
            case otf2::common::collective_type::scatterv:
                return {"scatterv"};
            case otf2::common::collective_type::all_gather:
                return {"all gather"};
            case otf2::common::collective_type::all_gatherv:
                return {"all gatherv"};
            case otf2::common::collective_type::all_to_all:
                return {"all to all"};
            case otf2::common::collective_type::all_to_allv:
                return {"all to allv"};
            case otf2::common::collective_type::all_to_allw:
                return {"all to allw"};
            case otf2::common::collective_type::all_reduce:
                return {"all reduce"};
            case otf2::common::collective_type::reduce_scatter:
                return {"reduce scatter"};
            case otf2::common::collective_type::scan:
                return {"scan"};
            case otf2::common::collective_type::exscan:
                return {"exscan"};
            case otf2::common::collective_type::reduce_scatter_block:
                return {"reduce scatter block"};
            case otf2::common::collective_type::create_handle:
                return {"create handle"};
            case otf2::common::collective_type::destroy_handle:
                return {"destroy handle"};
            case otf2::common::collective_type::allocate:
                return {"allocate"};
            case otf2::common::collective_type::deallocate:
                return {"deallocate"};
            case otf2::common::collective_type::create_handle_and_allocate:
                return {"create handle and allocate"};
            case otf2::common::collective_type::destroy_handle_and_deallocate:
                return {"destroy handle and deallocate"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::group_type& grp_type) noexcept
    {
        switch (grp_type)
        {
            case otf2::common::group_type::unknown:
                return {"unknown"};
            case otf2::common::group_type::locations:
                return {"locations"};
            case otf2::common::group_type::regions:
                return {"regions"};
            case otf2::common::group_type::metric:
                return {"metric"};
            case otf2::common::group_type::comm_locations:
                return {"comm locations"};
            case otf2::common::group_type::comm_group:
                return {"comm group"};
            case otf2::common::group_type::comm_self:
                return {"comm self"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::group_flag_type& grp_flag_type) noexcept
    {
        switch (grp_flag_type)
        {
            case otf2::common::group_flag_type::none:
                return {"none"};
            case otf2::common::group_flag_type::global_members:
                return {"global members"};
            default:
                return {"UNDEFINED"};
        }
    }

    std::string to_string(const otf2::common::paradigm_type& para_type) noexcept
    {
        switch (para_type)
        {
            case otf2::common::paradigm_type::unknown:
                return {"unknown"};
            case otf2::common::paradigm_type::user:
                return {"user"};
            case otf2::common::paradigm_type::compiler:
                return {"compiler"};
            case otf2::common::paradigm_type::openmp:
                return {"openmp"};
            case otf2::common::paradigm_type::mpi:
                return {"mpi"};
            case otf2::common::paradigm_type::cuda:
                return {"cuda"};
            case otf2::common::paradigm_type::measurement_system:
                return {"measurement system"};
            case otf2::common::paradigm_type::pthread:
                return {"pthread"};
            case otf2::common::paradigm_type::hmpp:
                return {"hmpp"};
            case otf2::common::paradigm_type::ompss:
                return {"ompss"};
            case otf2::common::paradigm_type::hardware:
                return {"hardware"};
            case otf2::common::paradigm_type::gaspi:
                return {"gaspi"};
            case otf2::common::paradigm_type::upc:
                return {"upc"};
            case otf2::common::paradigm_type::shmem:
                return {"shmem"};
            case otf2::common::paradigm_type::winthread:
                return {"winthread"};
            case otf2::common::paradigm_type::qtthread:
                return {"qtthread"};
            case otf2::common::paradigm_type::acethread:
                return {"acethread"};
            case otf2::common::paradigm_type::tbbthread:
                return {"tbbthread"};
            case otf2::common::paradigm_type::openacc:
                return {"openacc"};
            case otf2::common::paradigm_type::opencl:
                return {"opencl"};
            case otf2::common::paradigm_type::mtapi:
                return {"mtapi"};
            case otf2::common::paradigm_type::sampling:
                return {"sampling"};
            case otf2::common::paradigm_type::none:
                return {"none"};
            default:
                return {"UNDEFINED"};
        }
    }

    struct io_creation_option_container
    {
        otf2::common::io_status_flag_type status_flag;
        otf2::common::io_creation_flag_type creation_flag;
        boost::optional<otf2::common::io_access_mode_type> access_mode;

        io_creation_option_container(const otf2::common::io_status_flag_type& status,
                                     const otf2::common::io_creation_flag_type& flag,
                                     const otf2::common::io_access_mode_type& mode) noexcept
            : status_flag(status), creation_flag(flag), access_mode(mode)
        {
        }

        io_creation_option_container(const otf2::common::io_status_flag_type& status) noexcept
            : status_flag(status), creation_flag(otf2::common::io_creation_flag_type::none)
        {
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const io_creation_option_container& option)
    {
        try {
            return os << "status flag: " << to_string(option.status_flag)
                    << " creation flag: " << to_string(option.creation_flag)
                    << " access mode: " << to_string(option.access_mode.value());
        }
        catch (const boost::bad_optional_access& e)
        {
            logging::debug() << "BAD OPTIONAL ACCESS RAISED";
        }

        return os << "status flag: " << to_string(option.status_flag)
                << " creation flag: " << to_string(option.creation_flag);
    }

    struct io_operation_option_container
    {
        otf2::common::io_operation_mode_type op_mode;
        otf2::common::io_operation_flag_type op_flag;

        io_operation_option_container() = default;

        io_operation_option_container(const otf2::common::io_operation_mode_type& mode,
                                      const otf2::common::io_operation_flag_type& flag) noexcept
            : op_mode(mode), op_flag(flag)
        {
        }

        io_operation_option_container(const otf2::common::io_operation_mode_type& mode) noexcept
            : op_mode(mode), op_flag(otf2::common::io_operation_flag_type::none)
        {
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const io_operation_option_container& option)
    {
        return os << "operation mode: " << to_string(option.op_mode)
                << " operation flag: " << to_string(option.op_flag);
    }

    struct option_type_printer : boost::static_visitor<std::string>
    {
        template<typename OT>
        std::string operator()(const OT& option_type) const
        {
            std::stringstream s;
            s << option_type;
            return s.str();
        }

        std::string operator()(const otf2::common::io_seek_option_type& option_type) const
        {
            return to_string(option_type);
        }

    };

    struct vertex_io_event_property
    {
        using option_type = boost::variant<io_operation_option_container,
                                           io_creation_option_container,
                                           otf2::common::io_seek_option_type>;

        int proc_id;
        std::string filename;
        std::string region_name;
        std::uint64_t request_size; // bytes requested by an I/O operation
        std::uint64_t response_size; // bytes actually touched by this I/O operation
        std::uint64_t offset;
        option_type option;
        otf2::chrono::time_point timestamp;

        vertex_io_event_property() noexcept :  proc_id(-1), filename(""), region_name(""),
            request_size(0), response_size(0),  offset(0), option(), timestamp()
        {
        }

        vertex_io_event_property(int process_id, const std::string& fname,
                                 const std::string& reg_name, std::uint64_t req_size,
                                 std::uint64_t resp_size,
                                 std::uint64_t off, option_type mode,
                                 const otf2::chrono::time_point ts) noexcept
        : proc_id(process_id), filename(fname), region_name(reg_name), request_size(req_size),
            response_size(resp_size), offset(off), option(mode), timestamp(ts)
        {
        }

        ~vertex_io_event_property()
        {
        }

    };

    inline std::ostream& operator<<(std::ostream& os, const vertex_io_event_property& vertex)
    {
        return os << "process id: " << vertex.proc_id
                 << " filename: " << vertex.filename
                 << " region: " << vertex.region_name
                 << " request_size:  " << vertex.request_size
                 << " response size: " << vertex.response_size
                 << " offset: " << vertex.offset
                 << " mode: " << boost::apply_visitor(option_type_printer(), vertex.option)
                 << " timestamp: " << vertex.timestamp;
    }

    struct vertex_sync_event_property
    {
        int proc_id;
        std::string region_name;
        otf2::chrono::time_point timestamp;

        vertex_sync_event_property() noexcept : proc_id(-1), region_name(""), timestamp()
        {
        }

        vertex_sync_event_property(int process_id, const std::string& rname,
                                   const otf2::chrono::time_point ts) noexcept
        : proc_id(process_id), region_name(rname), timestamp(ts)
        {
        }

        ~vertex_sync_event_property()
        {
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const vertex_sync_event_property& vertex)
    {
        return os << "sync event\n"
                << "region: " << vertex.region_name
                << "timestamp: " << vertex.timestamp;
    }

    struct vertex_event_type
    {
        using vertex_property = boost::variant<vertex_io_event_property,
                                               vertex_sync_event_property>;
        vertex_kind type;
        vertex_property property;

        vertex_event_type() = default;

        vertex_event_type(const vertex_kind& t) noexcept : type(t)
        {
        }

        vertex_event_type(const vertex_io_event_property& io_p) : type(vertex_kind::io_event),
            property(io_p)
        {
        }

        vertex_event_type(const vertex_sync_event_property& sync_p) : type(vertex_kind::sync_event),
            property(sync_p)
        {
        }

        ~vertex_event_type()
        {
        }

        template<typename Archiver>
        void serialize(Archiver& ar, const unsigned int /* version */)
        {
            // TODO: Serialization is mandatory, do something useful here!
        }
    };

    using graph_impl = boost::adjacency_list<
                                boost::vecS,
                                boost::distributedS<boost::graph::distributed::mpi_process_group,
                                                    boost::vecS>,
                                boost::undirectedS, // NOTE: undirected!
                                vertex_event>; // vertex type

    using simple_graph_impl = boost::adjacency_list<
                                        boost::vecS,
                                        boost::distributedS<boost::graph::distributed::mpi_process_group,
                                                            boost::vecS>,
                                        boost::directedS, // NOTE: directed!
                                        vertex_event_type>;


    template<typename GraphImpl>
    class graph
    {
        public:
            using vertex_descriptor = typename boost::graph_traits<GraphImpl>::vertex_descriptor;
            using vertex_iterator = typename boost::graph_traits<GraphImpl>::vertex_iterator;
            using edge_descriptor = typename boost::graph_traits<GraphImpl>::edge_descriptor;
            using vertex_type = typename boost::vertex_bundle_type<GraphImpl>::type;
            using edge_type = typename boost::vertex_bundle_type<GraphImpl>::type;
            using vertex_range = std::pair<vertex_iterator, vertex_iterator>;
            using edge_add_t = std::pair<edge_descriptor, bool>;


            graph() noexcept : graph_(std::make_unique<GraphImpl>())
            {
            }

            ~graph()
            {
            }

            GraphImpl* get() noexcept
            {
                return graph_.get();
            }

            vertex_descriptor add_vertex(const vertex_type& v)
            {
                const auto vd = boost::add_vertex(v, *graph_.get());
                return vd;
            }

            edge_add_t add_edge(const vertex_descriptor& vd_from,
                                const vertex_descriptor& vd_to)
            {
                return boost::add_edge(vd_from, vd_to, *graph_.get());
            }

            vertex_range vertices()
            {
                return boost::vertices(*graph_.get());
            }

            std::size_t num_vertices() const
            {
                return boost::num_vertices(*graph_.get());
            }

            std::size_t num_edges() const
            {
                return boost::num_edges(*graph_.get());
            }

            vertex_type& operator[](const vertex_descriptor& vd) const
            {
                return graph_.get()->operator[](vd);
            }

        private:
            std::unique_ptr<GraphImpl> graph_;
    };

    using Graph = graph<graph_impl>;

    using SimpleGraph = graph<simple_graph_impl>;

    template<typename G>
    class vertex_event_writer
    {
        public:
            vertex_event_writer(G* graph_ptr) : g_ptr_(graph_ptr) // pass ptr, because we can copy ptrs
            {
            }

            template<typename vertex_descriptor>
            void operator()(std::ostream& os, const vertex_descriptor& vd) const
            {
                auto vertex = g_ptr_->operator[](vd);
                if (vertex.type == vertex_kind::io_event) {
                    auto property = boost::get<vertex_io_event_property>(vertex.property);
                    os << "[label=\"" << property.region_name
                        << "\", comment=\"" << property.proc_id << "\""
                        << "]";
                }
                else if (vertex.type == vertex_kind::sync_event) {
                    auto property = boost::get<vertex_sync_event_property>(vertex.property);
                    os << "[label=\"" << property.region_name
                        << "\", comment=\"" << property.proc_id << "\""
                        << "]";
                }
                else {
                    logging::fatal() << "Unrecognized vertex property for graphviz output";
                }
            }

        private:
            G* g_ptr_;
    };


    template<typename G>
    void write_graph_to_dot(G& graph, const std::string& filename)
    {
        std::ofstream file{filename};
        boost::write_graphviz(file, *graph.get(),
                make_vertex_event_writer(graph));
    }

    template<typename G>
    inline vertex_event_writer<G>
    make_vertex_event_writer(G& graph)
    {
        return vertex_event_writer<G>(&graph);
    }

} // namespace rabbitxx

#endif // __RABBITXX_GRAPH_HPP__

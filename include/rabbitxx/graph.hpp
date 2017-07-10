#ifndef __RABBITXX_GRAPH_HPP__
#define __RABBITXX_GRAPH_HPP__

#include <rabbitxx/log.hpp>

#include <nitro/lang/quaint_ptr.hpp>

#include <otf2xx/event/events.hpp>
#include <otf2xx/chrono/chrono.hpp>

#include <boost/graph/use_mpi.hpp>
#include <boost/variant.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>

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
    std::string to_string(const otf2::common::io_operation_mode_type& mode)
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
                return {"NONE"};
        }
    }

    // helper function, return string representation for io_seek_option_type
    std::string to_string(const otf2::common::io_seek_option_type& op_type)
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
                return {"NONE"};
        }
    }

    struct option_type_printer : boost::static_visitor<std::string>
    {
        template<typename OT>
        std::string operator()(const OT& option_type) const
        {
            return to_string(option_type);
        }
    };

    struct vertex_io_event_property
    {
        using option_type = boost::variant<otf2::common::io_operation_mode_type,
                                           otf2::common::io_seek_option_type>;

        int proc_id;
        std::string filename;
        std::string region_name;
        std::uint64_t request_size; // in bytes
        std::uint64_t offset;
        option_type option;
        otf2::chrono::time_point timestamp;

        vertex_io_event_property() noexcept :  proc_id(-1), filename(""), region_name(""),
            request_size(0), offset(0), option(), timestamp()
        {
        }

        vertex_io_event_property(int process_id, const std::string& fname,
                                 const std::string& reg_name, std::uint64_t req_size,
                                 std::uint64_t off, option_type mode,
                                 const otf2::chrono::time_point ts) noexcept
        : proc_id(process_id), filename(fname), region_name(reg_name), request_size(req_size),
            offset(off), option(mode), timestamp(ts)
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
                 << " offset: " << vertex.offset
                 << " mode: " << boost::apply_visitor(option_type_printer(), vertex.option)
                 << " timestamp: " << vertex.timestamp;
    }

    struct vertex_sync_event_property
    {
        vertex_sync_event_property() noexcept
        {
        }

        ~vertex_sync_event_property()
        {
        }

    };

    inline std::ostream& operator<<(std::ostream& os, const vertex_sync_event_property& vertex)
    {
        return os << "sync event vertex";
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
                //(*graph_.get())[vd] = v;
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

} // namespace rabbitxx

#endif // __RABBITXX_GRAPH_HPP__

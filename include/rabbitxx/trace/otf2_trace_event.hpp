#ifndef __RABBITXX_GRAPH_OTF2_TRACE_EVENT_HPP__
#define __RABBITXX_GRAPH_OTF2_TRACE_EVENT_HPP__

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/graph/graphviz.hpp>

#include <otf2xx/otf2.hpp>

#include <rabbitxx/log.hpp>
#include <utils/enum_to_string.hpp>

namespace rabbitxx {

    enum class vertex_kind
    {
        io_event,
        sync_event,
    };

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

    struct io_event_property
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

        io_event_property() noexcept :  proc_id(-1), filename(""), region_name(""),
            request_size(0), response_size(0),  offset(0), option(), timestamp()
        {
        }

        io_event_property(int process_id, const std::string& fname,
                            const std::string& reg_name, std::uint64_t req_size,
                            std::uint64_t resp_size,
                            std::uint64_t off, option_type mode,
                            const otf2::chrono::time_point ts) noexcept
        : proc_id(process_id), filename(fname), region_name(reg_name), request_size(req_size),
            response_size(resp_size), offset(off), option(mode), timestamp(ts)
        {
        }

        ~io_event_property()
        {
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const io_event_property& vertex)
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

    struct sync_event_property
    {
        int proc_id;
        std::string region_name;
        std::uint32_t root_rank;
        std::vector<std::uint64_t> members;
        otf2::chrono::time_point timestamp;
        // TODO here we need some tag defining which kind of synchronization happens. This is necessary since collective operations have another semantic
        // than recieve 

        sync_event_property() noexcept : proc_id(-1), region_name(""), root_rank(-1), members(), timestamp()
        {
        }

        sync_event_property(int process_id, const std::string& rname, std::uint32_t root,
                                   const std::vector<std::uint64_t>& member,
                                   const otf2::chrono::time_point ts) noexcept
        : proc_id(process_id), region_name(rname), root_rank(root), members(member), timestamp(ts)
        {
        }

        sync_event_property(const sync_event_property&) = default;
        sync_event_property& operator=(const sync_event_property&) = default;

        sync_event_property(sync_event_property&&) = default;
        sync_event_property& operator=(sync_event_property&&) = default;

        ~sync_event_property()
        {
        }

    };

    inline std::ostream& operator<<(std::ostream& os, const sync_event_property& vertex)
    {
        os << "sync event "
            << "region: " << vertex.region_name
            << " root: " << vertex.root_rank
            << " members: ";
        std::copy(vertex.members.begin(), vertex.members.end(),
                  std::ostream_iterator<std::uint64_t>(os, " "));
        os << " timestamp: " << vertex.timestamp;
        return os;
    }

    struct otf2_trace_event
    {
        using vertex_property = boost::variant<io_event_property,
                                               sync_event_property>;
        vertex_kind type;
        vertex_property property;

        otf2_trace_event() = default;

        otf2_trace_event(const vertex_kind& t) noexcept : type(t)
        {
        }

        otf2_trace_event(const io_event_property& io_p) noexcept
        : type(vertex_kind::io_event), property(io_p)
        {
        }

        otf2_trace_event(const sync_event_property& sync_p) noexcept 
        : type(vertex_kind::sync_event), property(sync_p)
        {
        }

        ~otf2_trace_event()
        {
        }

    };

    inline std::ostream& operator<<(std::ostream& os, const rabbitxx::otf2_trace_event& vertex)
    {
        if (vertex.type == vertex_kind::io_event) {
            return os << "io event:\n" << vertex.property;
        }
        else if (vertex.type == vertex_kind::sync_event)
        {
            return os << "sync event:\n" << vertex.property;
        }
    }

    template<typename G>
    class otf2_trace_event_writer
    {
        public:
            otf2_trace_event_writer(G* graph_ptr) : g_ptr_(graph_ptr) // pass ptr, because we can copy ptrs
            {
            }

            template<typename vertex_descriptor>
            void operator()(std::ostream& os, const vertex_descriptor& vd) const
            {
                auto vertex = g_ptr_->operator[](vd);
                if (vertex.type == vertex_kind::io_event) {
                    auto property = boost::get<io_event_property>(vertex.property);
                    const std::string label_str = build_label(property.region_name, property.proc_id);
                    os << "[label=\"" << label_str << "\""
                        << ", color=red"
                        << "]";
                }
                else if (vertex.type == vertex_kind::sync_event) {
                    auto property = boost::get<sync_event_property>(vertex.property);
                    const std::string label_str = build_label(property.region_name, property.proc_id);
                    os << "[label=\"" << label_str << "\""
                        << ", color=green"
                        << "]";
                }
                else {
                    logging::fatal() << "Unrecognized vertex property for graphviz output";
                }
            }

        private:
            const std::string build_label(const std::string& region_name, int rank) const
            {
                std::stringstream sstr;
                sstr << region_name << " @ " << rank;
                return sstr.str();
            }

        private:
            G* g_ptr_;
    };


    template<typename G>
    void write_graph_to_dot(G& graph, const std::string& filename)
    {
        std::ofstream file{filename};
        boost::write_graphviz(file, graph,
                make_otf2_trace_event_writer(graph));
    }

    template<typename G>
    inline otf2_trace_event_writer<G>
    make_otf2_trace_event_writer(G& graph)
    {
        return otf2_trace_event_writer<G>(&graph);
    }

} // namespace rabbtixx

#endif /* __RABBITXX_GRAPH_OTF2_TRACE_EVENT_HPP__ */

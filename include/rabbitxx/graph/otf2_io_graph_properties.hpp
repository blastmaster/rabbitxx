#ifndef RABBITXX_OTF2_IO_GRAPH_PROPERTIES_HPP
#define RABBITXX_OTF2_IO_GRAPH_PROPERTIES_HPP

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/graph/graphviz.hpp>

#include <otf2xx/otf2.hpp>

#include <rabbitxx/log.hpp>
#include <rabbitxx/utils.hpp>

#include <limits>

namespace rabbitxx {

/**
* Describes which kind of vertex the current `otf2_trace_event` are.
* `io_event` I/O vertex
* `sync_event` synchronization vertex
* `synthetic` synthetic vertex with no additional semantic meaning.
*/
enum class vertex_kind
{
    io_event,
    sync_event,
    synthetic,
    none,
};

inline std::ostream& operator<<(std::ostream& os, const vertex_kind& kind)
{
    switch (kind)
    {
        case vertex_kind::io_event:
            os << "io event"; break;
        case vertex_kind::sync_event:
            os << "sync event"; break;
        case vertex_kind::synthetic:
            os << "synthetic"; break;
        case vertex_kind::none:
            os << "None"; break;
    }
    return os;
}

// scope of a synchronization operation
// can be global which means that the operation covers all processes
// or local which mean that the operation does not cover all processes
enum class sync_scope
{
    Local,
    Global
};

inline std::ostream&
operator<<(std::ostream& os, const sync_scope& scope)
{
    switch (scope)
    {
        case sync_scope::Local:
            os << "Local";
            break;
        case sync_scope::Global:
            os << "Global";
            break;
        default:
            os << "NONE";
            break;
    }
    return os;
}


struct io_creation_option_container
{
    otf2::common::io_status_flag_type status_flag;
    otf2::common::io_creation_flag_type creation_flag;
    boost::optional<otf2::common::io_access_mode_type> access_mode = boost::none;

    explicit io_creation_option_container(const otf2::common::io_status_flag_type& status,
                                    const otf2::common::io_creation_flag_type& flag,
                                    const otf2::common::io_access_mode_type& mode) noexcept
        : status_flag(status), creation_flag(flag), access_mode(mode)
    {
    }

    explicit io_creation_option_container(const otf2::common::io_status_flag_type& status) noexcept
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

    explicit io_operation_option_container(const otf2::common::io_operation_mode_type& mode,
                                    const otf2::common::io_operation_flag_type& flag) noexcept
        : op_mode(mode), op_flag(flag)
    {
    }

    explicit io_operation_option_container(const otf2::common::io_operation_mode_type& mode) noexcept
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

enum class io_event_kind
{
    create,  // create or open a new handle
    dup,     // duplicate a handle
    seek,   // seek
    read,   // read operation
    write,  // write operation
    flush,  // flush
    delete_or_close, // delete or close handle
    none        // none
};

inline std::ostream& operator<<(std::ostream& os, const io_event_kind& io_kind)
{
    switch (io_kind)
    {
        case io_event_kind::create:
            os << "create";
            break;
        case io_event_kind::dup:
            os << "dup";
            break;
        case io_event_kind::seek:
            os << "seek";
            break;
        case io_event_kind::read:
            os << "read";
            break;
        case io_event_kind::write:
            os << "write";
            break;
        case io_event_kind::flush:
            os << "flush";
            break;
        case io_event_kind::delete_or_close:
            os << "close_or_delete";
            break;
        case io_event_kind::none:
            os << "None";
            break;
        default:
            logging::fatal() << "This should never happen! Wrong `io_event_kind`!";
            os << "UNDEFINED";
            break;
    }
    return os;
}

struct io_event_property
{
    using option_type = boost::variant<io_operation_option_container,
                                        io_creation_option_container,
                                        otf2::common::io_seek_option_type>;

    //TODO enum class to distinguish the different operation types
    //or a proper static visitor to check for which type we have
    // maybe {create_op, seek_op, read_op, write_op}

    std::uint64_t proc_id = std::numeric_limits<std::uint64_t>::max();
    std::string filename;
    std::string region_name;
    std::string paradigm;
    std::uint64_t request_size {0}; // bytes requested by an I/O operation
    std::uint64_t response_size {0}; // bytes actually touched by this I/O operation
    std::uint64_t offset {0};
    option_type option;
    io_event_kind kind;
    otf2::chrono::time_point timestamp;

    io_event_property() = default;

    explicit io_event_property(
                        std::uint64_t process_id,           /* pid */
                        const std::string& fname,           /* filename */
                        const std::string& reg_name,        /* region_name */
                        const std::string& paradigm,        /* paradigm */
                        std::uint64_t req_size,             /* request_size */
                        std::uint64_t resp_size,            /* response_size */
                        std::uint64_t off,                  /* offset */
                        option_type mode,                   /* operation mode */
                        io_event_kind event_kind,           /* operation kind */
                        const otf2::chrono::time_point ts) /* timestamp */ noexcept
    : proc_id(process_id), filename(fname), region_name(reg_name), paradigm(paradigm), request_size(req_size),
        response_size(resp_size), offset(off), option(mode), kind(event_kind), timestamp(ts)
    {
    }
};

inline std::ostream& operator<<(std::ostream& os, const io_event_property& vertex)
{
    return os << "process id: " << vertex.proc_id << "\n"
                << "filename: " << vertex.filename << "\n"
                << "region: " << vertex.region_name << "\n"
                << "paradigm: " << vertex.paradigm << "\n"
                << "request_size:  " << vertex.request_size << "\n"
                << "response size: " << vertex.response_size << "\n"
                << "offset: " << vertex.offset << "\n"
                << "mode: " << boost::apply_visitor(option_type_printer(), vertex.option) << "\n"
                << "kind: " << vertex.kind << "\n"
                << "timestamp: " << vertex.timestamp << "\n";
}

enum class sync_event_kind
{
    collective,
    p2p,
    async,      //FIXME: does not exist!
    none,
};

class peer2peer
{
public:
    static constexpr sync_event_kind kind()
    {
        return sync_event_kind::p2p;
    }

    explicit peer2peer(std::uint32_t rproc, std::uint32_t mtag, std::uint64_t mlength) noexcept
    : remote_process_(rproc), msg_tag_(mtag), msg_length_(mlength)
    {
    }

    explicit peer2peer(std::uint32_t rproc, std::uint32_t mtag, std::uint64_t mlength, std::uint64_t reqid) noexcept
    : remote_process_(rproc), msg_tag_(mtag), msg_length_(mlength), request_id_(reqid)
    {
    }

    // the other end of the communication in receive calls the sender in send calls the receiver
    std::uint32_t remote_process() const noexcept
    {
        return remote_process_;
    }

    std::uint32_t msg_tag() const noexcept
    {
        return msg_tag_;
    }

    std::uint64_t msg_length() const noexcept
    {
        return msg_length_;
    }

    boost::optional<std::uint64_t> request_id() const noexcept
    {
        return request_id_;
    }

private:
    std::uint32_t remote_process_ = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t msg_tag_ = std::numeric_limits<std::uint32_t>::max();
    std::uint64_t msg_length_ = std::numeric_limits<std::uint64_t>::max();
    boost::optional<std::uint64_t> request_id_;
};

class collective
{
public:
    static constexpr sync_event_kind kind()
    {
        return sync_event_kind::collective;
    }

    explicit collective(std::uint32_t root, std::vector<std::uint64_t> members) noexcept
    : root_rank_(root), members_(std::move(members))
    {
    }

    explicit collective(std::vector<std::uint64_t> members) noexcept
    : members_(std::move(members))
    {
    }

    bool has_root() const noexcept
    {
        return root_rank_ < std::numeric_limits<std::uint32_t>::max();
    }

    std::uint32_t root() const noexcept
    {
        return root_rank_;
    }

    std::vector<std::uint64_t> members() const noexcept
    {
        return members_;
    }

private:
    std::uint32_t root_rank_ = std::numeric_limits<std::uint32_t>::max();
    std::vector<std::uint64_t> members_;
};

struct sync_event_property
{
    using comm_type = boost::variant<peer2peer, collective>;

    std::uint64_t proc_id = std::numeric_limits<std::uint64_t>::max();
    std::string region_name;
    sync_event_kind comm_kind = sync_event_kind::none;
    comm_type op_data;
    otf2::chrono::time_point timestamp;
    size_t root_event = std::numeric_limits<size_t>::max();

    explicit sync_event_property(std::uint64_t process_id, const std::string& rname, const peer2peer& op_dat, const otf2::chrono::time_point ts) noexcept
    : proc_id(process_id), region_name(rname), comm_kind(sync_event_kind::p2p), op_data(op_dat), timestamp(ts)
    {
    }

    explicit sync_event_property(std::uint64_t process_id, const std::string& rname, const collective& op_dat, const otf2::chrono::time_point ts) noexcept
    : proc_id(process_id), region_name(rname), comm_kind(sync_event_kind::collective), op_data(op_dat), timestamp(ts)
    {
    }
};

struct comm_type_printer : boost::static_visitor<std::string>
{
    std::string operator()(const collective& comm_data) const
    {
        std::stringstream sstr;
        sstr << "[Collective-Event]\n";
        if (comm_data.has_root()) {
            sstr << "[Root rank]: " << comm_data.root() << "\n";
        }
        //FIXME
        //sstr << "[Members]: ";
        //std::copy(comm_data.members().begin(),
                    //comm_data.members().end(),
                    //std::ostream_iterator<std::uint64_t>(sstr, ", "));
        sstr << "[Member Size]: " << comm_data.members().size() << "\n";
        return sstr.str();
    }

    std::string operator()(const peer2peer& comm_data) const
    {
        std::stringstream sstr;
        sstr << "[P2P-Event]\n";
        sstr << "[Process]: " << comm_data.remote_process() << "\n"
            << "[Message tag]: " << comm_data.msg_tag() << "\n"
            << "[Message length]: " << comm_data.msg_length() << "\n";
        return sstr.str();
    }
};

inline std::ostream& operator<<(std::ostream& os, const sync_event_property& vertex)
{
    os << "[Sync-Event]\n"
        << "[Process ID]: " << vertex.proc_id << "\n"
        << "[Region Name]: " << vertex.region_name << "\n"
        << "[Kind]: "
        << (vertex.comm_kind == sync_event_kind::p2p ? "p2p" : "collective") << "\n"
        << boost::apply_visitor(comm_type_printer(), vertex.op_data) << "\n"
        << "[Timestamp]: " << vertex.timestamp << "\n";
    return os;
}

struct synthetic_event_property
{
    std::string name;
    otf2::chrono::time_point timestamp;

    explicit synthetic_event_property() noexcept : name("synthetic event")
    {
    }

    explicit synthetic_event_property(const std::string& evt_name, const otf2::chrono::time_point& ts) noexcept 
        : name(evt_name), timestamp(ts)
    {
    }

};

inline std::ostream& operator<<(std::ostream& os, const synthetic_event_property& vertex)
{
    os << "[Synthetic-Event]\n"
        << "[Name]: " << vertex.name << "\n"
        << "[Timestamp]: " << vertex.timestamp << "\n";
    return os;
}

struct timespan
{
    otf2::chrono::duration duration = otf2::chrono::duration(0);
    otf2::chrono::time_point enter = otf2::chrono::armageddon();
    otf2::chrono::time_point leave = otf2::chrono::armageddon();
};

inline std::ostream& operator<<(std::ostream& os, const timespan& ts)
{
    os << "Enter: " << ts.enter << "\n"
        << "Leave: " << ts.leave << "\n"
        << "Duration: " << ts.duration << "\n";
    return os;
}

/**
    * TODO:
    * Could it be clever to inherit from boost::variant<io_event_property, sync_event_property> ???
    * This should provide access to the variant api without extra effort and we could extend the class
    * for the vertex_kind functionality?
    */
struct otf2_trace_event
{
    using vertex_property = boost::variant<io_event_property,
                                            sync_event_property,
                                            synthetic_event_property>;
    vertex_kind type = vertex_kind::none;
    vertex_property property;
    //otf2::chrono::duration duration = otf2::chrono::duration(0);
    timespan duration;

    otf2_trace_event() = default;

    explicit otf2_trace_event(const vertex_kind& t)  noexcept : type(t)
    {
    }

    explicit otf2_trace_event(const io_event_property& io_p) noexcept
        : type(vertex_kind::io_event), property(io_p)
    {
    }

    explicit otf2_trace_event(const sync_event_property& sync_p) noexcept
        : type(vertex_kind::sync_event), property(sync_p)
    {
    }

    explicit otf2_trace_event(const synthetic_event_property& synthetic_p) noexcept
        : type(vertex_kind::synthetic), property(synthetic_p)
    {
    }

    std::uint64_t id() const
    {
        if (type == vertex_kind::io_event) {
            const auto& p = boost::get<io_event_property>(property);
            return p.proc_id;
        }
        if (type == vertex_kind::sync_event) {
            const auto& p = boost::get<sync_event_property>(property);
            return p.proc_id;
        }
        if (type == vertex_kind::synthetic) {
            logging::debug() << "Synthetic Event has no process id return INT MAX.";
            return std::numeric_limits<std::uint64_t>::max();
        }

        logging::fatal() << "This should not happen! property type seems wether of type io_event_property neither of type sync_event_property.";
        return std::numeric_limits<std::uint64_t>::max();
    }

    std::string name() const
    {
        if (type == vertex_kind::io_event) {
            const auto& p = boost::get<io_event_property>(property);
            return p.region_name;
        }
        if (type == vertex_kind::sync_event) {
            const auto& p = boost::get<sync_event_property>(property);
            return p.region_name;
        }
        if (type == vertex_kind::synthetic) {
            const auto& p = boost::get<synthetic_event_property>(property);
            return p.name;
        }
        logging::fatal() << "This should not happen! property type seems wether of type io_event_property neither of type sync_event_property.";
        return "";
    }

    otf2::chrono::time_point timestamp() const
    {
        if (type == vertex_kind::io_event) {
            const auto& p = boost::get<io_event_property>(property);
            return p.timestamp;
        }
        if (type == vertex_kind::sync_event) {
            const auto& p = boost::get<sync_event_property>(property);
            return p.timestamp;
        }
        if (type == vertex_kind::synthetic) {
            const auto& p = boost::get<synthetic_event_property>(property);
            return p.timestamp;
        }
        logging::fatal() << "ERROR invalid vertex_kind! Return MAX";
        return otf2::chrono::time_point::max();
    }

};

inline std::ostream& operator<<(std::ostream& os, const rabbitxx::otf2_trace_event& vertex)
{
    switch (vertex.type)
    {
        case vertex_kind::io_event:
            os << vertex.property;
            break;
        case vertex_kind::sync_event:
            os << vertex.property;
            break;
        case vertex_kind::synthetic:
            os << vertex.property;
            break;
        case vertex_kind::none:
            os << "None";
            break;
    }
    os << "Event duration: " << vertex.duration << "\n";
    return os;
}

/**
 * Graph property class, stores information about the overall program that are
 * gathered during construction.
 */
struct app_info
{
    otf2::chrono::duration total_time = otf2::chrono::duration(0);
    otf2::chrono::duration io_time = otf2::chrono::duration(0);
    otf2::chrono::duration io_metadata_time = otf2::chrono::duration(0);
    otf2::chrono::time_point first_event_time = otf2::chrono::genesis();
    otf2::chrono::time_point last_event_time = otf2::chrono::armageddon();
    otf2::definition::clock_properties clock_props;
    std::map<std::string, std::string> file_to_fs;
    std::uint64_t num_locations;
};

inline std::ostream& operator<<(std::ostream& os, const app_info& info)
{
    os << "total time: "
        << std::chrono::duration_cast<otf2::chrono::microseconds>(info.total_time) << "\n";
    os << "total file io time: "
        << std::chrono::duration_cast<otf2::chrono::microseconds>(info.io_time) << "\n";
    os << "total file io metadata time: "
        << std::chrono::duration_cast<otf2::chrono::microseconds>(info.io_metadata_time) << "\n";

    os << "first event time: " << info.first_event_time << "\n";
    os << "last event time: " << info.last_event_time << "\n";
    os << "first event time duration: "
        << std::chrono::duration_cast<otf2::chrono::microseconds>(info.first_event_time.time_since_epoch()) << "\n";
    os << "last event time duration: "
        << std::chrono::duration_cast<otf2::chrono::microseconds>(info.last_event_time.time_since_epoch()) << "\n";

    os << "ticks per second: "
        << info.clock_props.ticks_per_second().count() << "\n";
    os << "start time: "
        << info.clock_props.start_time().count() << "\n";
    os << "length: "
        << info.clock_props.length().count() << "\n";

    os << "File 2 Fs mapping\n";
    for (const auto& kvp : info.file_to_fs)
    {
        os << kvp.first << " -> " << kvp.second << "\n";
    }

    return os;
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
                const auto property = boost::get<io_event_property>(vertex.property);
                const std::string label_str = build_label(property.region_name, property.proc_id);
                os << R"([label=")" << "# " << vd << label_str << R"(")"
                    << ", color=red"
                    << "]";
            }
            else if (vertex.type == vertex_kind::sync_event) {
                const auto property = boost::get<sync_event_property>(vertex.property);
                const std::string label_str = build_label(property.region_name, property.proc_id);
                os << R"([label=")" << "# " << vd << label_str << R"(")"
                    << ", color=green"
                    << "]";
            }
            else if (vertex.type == vertex_kind::synthetic) {
                    logging::debug() << "Draw synthetic node!";
                const auto property = boost::get<synthetic_event_property>(vertex.property);
                os << R"([label=")" << property.name << R"(", color=gray])";
            }
            else {
                logging::fatal() << "Unrecognized vertex property for graphviz output";
            }
        }

    private:
        const std::string build_label(const std::string& region_name, int rank) const
        {
            std::stringstream sstr;
            sstr << " " << region_name << " @ " << rank;
            return sstr.str();
        }

    private:
        G* g_ptr_;
};

template<typename Set>
class set_graph_writer
{
    public:
        explicit set_graph_writer(const std::vector<Set>& set_v) : sets_(set_v) {}

        void operator()(std::ostream& out)
        {
            std::uint64_t s_cnt = 0;
            std::string color = colors_.front();
            for (const auto& set : sets_)
            {
                ++s_cnt;
                out << "subgraph cluster_" << s_cnt << "{\n"
                    << "shape=rect;\n"
                    << "style=filled;\n"
                    << "color=" << color << ";\n"
                    << R"(label=")" << "set_" << s_cnt << "\";\n";

                out << set.start_event() << " -> { ";
                std::copy(set.begin(), set.end(),
                        std::ostream_iterator<typename Set::value_type>(out, " "));
                out << " }"
                    << "[style=invis]"
                    << "\n}\n";
                color = colors_[s_cnt % colors_.size()];
            }
        }

    private:
        std::vector<Set> sets_;
        std::vector<std::string> colors_ {"lightblue", "springgreen", "orange", "lightgoldenrod"};
};

template<typename G>
void write_graph_to_dot(G& graph, const std::string& filename)
{
    std::ofstream file{filename};
    boost::write_graphviz(file, graph,
            make_otf2_trace_event_writer(graph));
}

template<typename Graph, typename Set>
void write_graph_to_dot(Graph& graph, const std::string& filename,
        const std::vector<Set>& sets)
{
    std::ofstream file{filename};
    boost::write_graphviz(file, graph,
            make_otf2_trace_event_writer(graph),
            boost::default_writer(),
            make_set_graph_writer(sets));
}

template<typename Set>
inline set_graph_writer<Set>
make_set_graph_writer(const std::vector<Set>& sets)
{
    return set_graph_writer<Set>(sets);
}

template<typename G>
inline otf2_trace_event_writer<G>
make_otf2_trace_event_writer(G& graph)
{
    return otf2_trace_event_writer<G>(&graph);
}

} // namespace rabbtixx

#endif /* RABBITXX_OTF2_IO_GRAPH_PROPERTIES_HPP */

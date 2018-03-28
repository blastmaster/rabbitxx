#include <rabbitxx/log.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/cio_stats.hpp>

#include <iostream>

using namespace rabbitxx;

/**
 * Request size statistic
 *
 *
 **/

//class FileStats
//{

//public:
    //FileStats() = default;

    //explicit FileStats(std::uint64_t pid, fs::path fname, std::vector<VertexDescriptor> evts) :
        //proc_id(pid), filename(fname), phase_evts(std::move(evts))
    //{
    //}

    //explicit FileStats(std::uint64_t pid, std::string fname, std::vector<VertexDescriptor> evts) :
        //proc_id(pid), filename(fname), phase_evts(std::move(evts))
    //{
    //}

    //std::uint64_t pid() const noexcept
    //{
        //return proc_id;
    //}

    //fs::path filename() const noexcept
    //{
        //return filename;
    //}

    //std::vector<VertexDescriptor>
    //events() const noexcept
    //{
        //return phase_evts;
    //}

    //bool empty() const noexcept { return phase_evts.empty(); }
    //std::size_t size() const noexcept { return phase_evts.size(); }

//private:
    //std::uint64_t proc_id;
    //fs::path filename;
    //std::vector<VertexDescriptor> phase_evts;
//};

//std::uint64_t
//get_request_size(const IoGraph& graph, VertexDescriptor vd)
//{
    //auto io_evt = get_io_property(graph, vd);
    //return io_evt.request_size;
//}

//void
//request_sizes_by_region(const IoGraph& graph, const set_t<VertexDescriptor>& cio_set)
//{
    //CIO_Stats stats(graph, cio_set);
    //auto kmap = kind_map(graph, cio_set);

//}

//void
//rw_per_file(const IoGraph& graph, const std::vector<VertexDescriptor>& rw_evts)
//{
    //for 

//}

//void
//request_sizes_per_file_per_process(const IoGraph& graph, set_t<VertexDescriptor>& cio_set)
//{
    //const auto rw_events = get_io_events_by_kind(graph, cio_set, { io_event_kind::read, io_event_kind::write });
    //rw_per_file(graph, rw_events);

//}

//void
//per_process(const IoGraph& graph)
//{
    //auto cio_sets_pp = rabbitxx::cio_sets_per_process(*graph);
//}

//void
//global(const IoGraph& graph)
//{
    //auto cio_sets = rabbitxx::find_cio_sets(*graph);
    //print_cio_set_stats(*graph, cio_sets);

//}

void print_header()
{
    std::cout << "Process-Id Request-Size Duration" << "\n";
}

void request_size_column(const rabbitxx::otf2_trace_event& evt)
{
    if (evt.type != rabbitxx::vertex_kind::io_event) {
        return;
    }
    auto io_evt = boost::get<rabbitxx::io_event_property>(evt.property);
    if (io_evt.kind == rabbitxx::io_event_kind::read || io_evt.kind == rabbitxx::io_event_kind::write)
    {
        if (io_evt.filename != "stdout") // FIXME URGHHSS
        {
            std::cout << io_evt.proc_id << " " << io_evt.request_size << " "
                << evt.duration.duration << " " << io_evt.region_name << " "
                << io_evt.kind << "\n";
        }
    }
}

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: " << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trc_file);
    auto vip = graph->vertices();
    print_header();
    for (auto it = vip.first; it != vip.second; ++it)
    {
        auto evt = graph->operator[](*it);
        request_size_column(evt);
    }

    return EXIT_SUCCESS;
}

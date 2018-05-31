#include <rabbitxx/cio_stats.hpp>

/**
 * Module to serialize sets to csv
 * This module prints all io events within a set into csv.
 *
 * TODO
 * * options have different length and are therefore omitted at the moment 
 * * header() does not work, one comma to much at the end.
 */

using namespace rabbitxx;

//TODO unused, one comma to much.
void header(std::ostream& os=std::cout)
{
    std::array<const char*, 10> columns {
        "pid",
        "filename",
        "region_name",
        "paradigm",
        "request_size",
        "response_size",
        "offset",
        //"option",
        "kind",
        "timestamp"};
    std::copy(columns.begin(), columns.end(), std::ostream_iterator<const char*>(os, ", "));
    os << "\n";
}

struct option_csv_printer : boost::static_visitor<std::string>
{
    std::string operator()(const io_operation_option_container& option) const
    {
        std::stringstream sstr;
        sstr << to_string(option.op_mode) << ", "
            << to_string(option.op_flag);

        return sstr.str();
    }

    std::string operator()(const io_creation_option_container& option) const
    {
        std::stringstream sstr;
        sstr << to_string(option.status_flag) << ", "
            << to_string(option.creation_flag) << ", ";
        if (option.access_mode)
        {
            sstr << to_string(option.access_mode.value()) << ", ";
        }

        return sstr.str();
    }

    std::string operator()(const otf2::common::io_seek_option_type& option) const
    {
        return to_string(option);
    }
};

std::ostream& io_event_2_csv_stream(const IoGraph& graph, const VertexDescriptor& evt, std::ostream& out)
{
    const auto& io_evt = get_io_property(graph, evt);
    out << io_evt.proc_id << ", "
        << io_evt.filename << ", "
        << io_evt.region_name << ", "
        << io_evt.paradigm << ", "
        << io_evt.request_size << ", "
        << io_evt.response_size << ", "
        << io_evt.offset << ", "
        //<< boost::apply_visitor(option_csv_printer(), evt.option) << ", "
        << io_evt.kind << ", "
        //<< graph[evt].duration.duration << ", "
        << otf2::chrono::duration_cast<otf2::chrono::nanoseconds>(graph[evt].duration.duration).count() << ", "
        << io_evt.timestamp;

    return out;
}

void set2csv(const IoGraph& graph, const set_t<VertexDescriptor>& set, std::ostream& out=std::cout)
{
    for (auto evt : set)
    {
        io_event_2_csv_stream(graph, evt, out);
        out << "\n";
    }

}

static std::string create_filename(std::uint64_t sidx)
{
    std::stringstream sstr;
    sstr << "set-" << sidx << ".csv";

    return sstr.str();
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error!\nUsage: ./" << argv[0] << " <trace-file>\n";
        return EXIT_FAILURE;
    }

    std::string trc_file(argv[1]);
    auto graph = make_graph<graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = find_cio_sets(graph);

    //header();
    std::uint64_t idx = 1;
    for (const auto& set : cio_sets)
    {
        std::string filename = create_filename(idx);
        std::ofstream ofile(filename);
        set2csv(graph, set, ofile);
        ++idx;
    }

    return EXIT_SUCCESS;
}

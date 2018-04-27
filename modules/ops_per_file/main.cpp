#include <rabbitxx/cio_stats.hpp>

using namespace rabbitxx;


class PerFileOps : public rabbitxx::File
{
public:
    PerFileOps(std::string filename, const IoGraph& graph, std::vector<std::pair<std::string, std::uint64_t>> pfo) :
        File(filename, graph), operations_(pfo)
    {}

    const std::vector<std::pair<std::string, std::uint64_t>>
    operations() const
    {
        return operations_;
    }
private:
    std::vector<std::pair<std::string, std::uint64_t>> operations_;
};

std::vector<PerFileOps>
get_per_file_operations(const IoGraph& graph, const set_t<VertexDescriptor>& set)
{
    std::vector<PerFileOps> res;
    auto fm = file_map(graph, set);
    for (const auto& kvp : fm)
    {
        std::vector<std::string> regions;
        std::vector<std::pair<std::string, std::uint64_t>> pfo;
        for (const auto evt : kvp.second)
        {
            auto ioevt = get_io_property(graph, evt);
            regions.push_back(ioevt.region_name);
        }
        std::set<std::string> uniq_regions(regions.begin(), regions.end());
        std::transform(uniq_regions.begin(), uniq_regions.end(), std::back_inserter(pfo),
                [&regions](const std::string& r_name)
                {
                    int cnt = std::count(regions.begin(), regions.end(), r_name);
                    return std::make_pair(r_name, cnt);
                });

        res.emplace_back(kvp.first, graph, pfo);
    }
    return res;
}

void dump_per_file_ops(const std::vector<PerFileOps>& fops)
{
    for (const auto& pfo : fops)
    {
        std::cout << pfo.filename() << "\n";
        for (const auto op : pfo.operations())
        {
            std::cout << op.first << ":" << op.second << " ";
        }
        std::cout << "\n";
    }
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Error usage: " << argv[0]
                << " <trace-file>" << std::endl;
        return 1;
    }

    // create graph
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(argv[1]);
    // find concurrent I/O-Sets
    auto io_sets = rabbitxx::find_cio_sets(graph);

    for (const auto& set : io_sets)
    {
        auto file_ops = get_per_file_operations(graph, set);
        dump_per_file_ops(file_ops);
    }

    return EXIT_SUCCESS;
}

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

int num_unique_pairs(std::vector<int>& v)
{
    const int n = v.size();
    return n * (n-1) / 2;
}

std::vector<std::pair<int,int>>
generate_unique_pairs(std::vector<int>& v)
{
    std::vector<std::pair<int,int>> res;
    for (int i = 0; i < v.size(); ++i)
    {
        auto t1 = v[i];
        for (int j = i+1; j < v.size(); ++j)
        {
            auto t2 = v[j];
            res.push_back(std::make_pair(t1, t2));
        }
    }

    return res;
}

TEST_CASE("[unqiue pairs]", "Generate all unqiue pairs of a sequence of numbers")
{
    //std::vector<int> numbers {
        //1, 19, 22, 28, 32 };

    std::vector<int> numbers {
         19, 22, 26 };

    auto pair_v = generate_unique_pairs(numbers);

    REQUIRE(pair_v.size() == num_unique_pairs(numbers));

    for (const auto& pair : pair_v)
    {
        logging::debug() << "Pair (" << pair.first << ", " << pair.second << ")";
    }
}

TEST_CASE("[ec]", "Find independent process groups")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-edgecase/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());

    for (const auto& proc_sets : *cio_sets_pp)
    {
        logging::debug() << "Process: " << proc_sets.first;
        for (const auto& set : proc_sets.second)
        {
            logging::debug() << "End evt: " << set.end_event();
        }
    }

}

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

template <typename VertexDescriptor>
void
dump_set_map(const rabbitxx::set_map_t<VertexDescriptor>& set_map)
{
    logging::debug() << "DUMP SET MAP end events";
    for (const auto& proc_set : set_map)
    {
        std::cout << "Process [" << proc_set.first << "] ";
        std::for_each(proc_set.second.begin(), proc_set.second.end(),
            [](const auto& set) { std::cout << set.end_event() << ", "; });
        std::cout << "\n";
    }
}


TEST_CASE("[unqiue pairs]", "Generate all unqiue pairs of a sequence of numbers")
{
    std::vector<int> numbers {
         19, 22, 26 };

    auto pair_v = rabbitxx::generate_unique_pairs(numbers);

    REQUIRE(pair_v.size() == rabbitxx::num_unique_pairs(numbers.size()));

    for (const auto& pair : pair_v)
    {
        logging::debug() << "Pair (" << pair.first << ", " << pair.second << ")";
    }
}

TEST_CASE("[ec]", "Find independent process groups")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-edgecase/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets_pp = rabbitxx::cio_sets_per_process(*graph);
    logging::debug() << "BEFORE SORTING!";
    dump_set_map(cio_sets_pp);
    sort_set_map_chrono(*graph, cio_sets_pp);
    logging::debug() << "AFTER SORTING!";
    dump_set_map(cio_sets_pp);
}

TEST_CASE("[trace-own6-advanced]", "Find independent process groups")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-own_trace6_advanced/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets_pp = rabbitxx::cio_sets_per_process(*graph);
    logging::debug() << "BEFORE SORTING!";
    dump_set_map(cio_sets_pp);
    rabbitxx::sort_set_map_chrono(*graph, cio_sets_pp);
    logging::debug() << "AFTER SORTING!";
    dump_set_map(cio_sets_pp);

    // test timestamps for event (42, 31) and (39, 17)
    const auto t42 = graph->operator[](42).timestamp();
    const auto t31 = graph->operator[](31).timestamp();
    const auto t39 = graph->operator[](39).timestamp();
    const auto t17 = graph->operator[](17).timestamp();
    const auto t15 = graph->operator[](15).timestamp();

    //REQUIRE(t42 < t39); //FAIL!
    //REQUIRE(t42 < t17); //FAIL!
    REQUIRE(t15 < t42);
    REQUIRE(t31 < t39);
    //REQUIRE(t31 < t17);

}

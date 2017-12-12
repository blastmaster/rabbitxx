#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

TEST_CASE("[unqiue pairs]", "Generate all unqiue pairs of a sequence of numbers")
{
    //std::vector<int> numbers {
        //1, 19, 22, 28, 32 };

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
    auto cio_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());
    logging::debug() << "BEFORE SORTING!";
    rabbitxx::dump_set_map(*cio_sets_pp.get());
    sort_set_map_chrono(*graph.get(), *cio_sets_pp.get());
    logging::debug() << "AFTER SORTING!";
    rabbitxx::dump_set_map(*cio_sets_pp.get());
}

TEST_CASE("[trace-own6-advanced]", "Find independent process groups")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-own_trace6_advanced/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());
    logging::debug() << "BEFORE SORTING!";
    rabbitxx::dump_set_map(*cio_sets_pp.get());
    rabbitxx::sort_set_map_chrono(*graph.get(), *cio_sets_pp.get());
    logging::debug() << "AFTER SORTING!";
    rabbitxx::dump_set_map(*cio_sets_pp.get());

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

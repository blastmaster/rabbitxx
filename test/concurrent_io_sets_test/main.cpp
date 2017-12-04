#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

TEST_CASE("[trace-simple]", "Find concurrent I/O sets")
{
    static const std::string trc_file {"/home/soeste/traces/dios/24.10/trace-simple/traces.otf2"};

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = rabbitxx::gather_concurrent_io_sets(*graph.get());

    static const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts_s1 {
        2, 4, 5, 6 };

    static const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts_s2 {
        11, 12, 13, 14, 15, 16 };

    REQUIRE(cio_sets.size() == 2);

    REQUIRE(cio_sets.front().size() == exp_evts_s1.size());
    REQUIRE(cio_sets.back().size() == exp_evts_s2.size());

    // check if first set matches exp_evts_s1
    REQUIRE(std::all_of(cio_sets.front().begin(),
                cio_sets.front().end(),
                [](const auto& evt) {// no need for capture since exp_evts_s1 is static
                    return std::any_of(exp_evts_s1.begin(),
                                     exp_evts_s1.end(),
                                     [&evt](const auto& e_evt) {
                                        return e_evt == evt;
                                     });
                    }));

    // check if second set matches exp_evts_s2
    REQUIRE(std::all_of(cio_sets.back().begin(),
                cio_sets.back().end(),
                [](const auto& evt) {// no need for capture since exp_evts_s2 is static
                    return std::any_of(exp_evts_s2.begin(),
                                     exp_evts_s2.end(),
                                     [&evt](const auto& e_evt) {
                                        return e_evt == evt;
                                     });
                    }));
}

TEST_CASE("[trace-own]", "Find concurrent I/O sets")
{
    static const std::string trc_file {"/home/soeste/traces/trace-own_trace-20171116_1704_50058813361150354/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = rabbitxx::gather_concurrent_io_sets(*graph.get());

    const std::vector<
        std::set<typename decltype(graph)::element_type::vertex_descriptor>>
        exp_sets {
            { 5, 6, 7, 14 },
            { 7, 10, 13, 14 },
            { 7, 10, 16 },
            { 10, 11, 16, 19 }
        };

    REQUIRE(cio_sets.size() == exp_sets.size());

    for (std::size_t i = 0; i < cio_sets.size(); ++i)
    {
        REQUIRE(std::all_of(cio_sets[i].begin(),
                cio_sets[i].end(),
                [&exp_sets, &i](const auto& evt) {
                    return std::any_of(exp_sets[i].begin(),
                            exp_sets[i].end(),
                            [&evt](const auto& e_evt) {
                                return evt == e_evt;
                            });
                }));
    }
}

TEST_CASE("[ec]", "Find concurrent I/O sets")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-edgecase/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = rabbitxx::gather_concurrent_io_sets(*graph.get());

    const std::vector<
        std::set<typename decltype(graph)::element_type::vertex_descriptor>>
        exp_sets {
            { 5, 6, 7, 8 },
            { 5, 7, 15, 25 },
            { 5, 7, 17, 18, 27 },
            { 6, 8, 11, 20 },
            { 6, 8, 13, 14, 23, 24 },
            { 11, 15, 20, 25 },
            { 11, 17, 18, 20, 27 },
            { 13, 14, 15, 23, 24, 25 },
            { 13, 14, 17, 18, 23, 24, 27 }
        };

    REQUIRE(cio_sets.size() == exp_sets.size());

    for (std::size_t i = 0; i < cio_sets.size(); ++i)
    {
        REQUIRE(std::all_of(cio_sets[i].begin(),
                cio_sets[i].end(),
                [&exp_sets, &i](const auto& evt) {
                    return std::any_of(exp_sets[i].begin(),
                            exp_sets[i].end(),
                            [&evt](const auto& e_evt) {
                                return evt == e_evt;
                            });
                }));
    }
}

TEST_CASE("[ech]", "Find concurrent I/O sets")
{
    static const std::string trc_file {"/home/soeste/traces/dios/edge_case_hard/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = rabbitxx::gather_concurrent_io_sets(*graph.get());

    const std::vector<
        std::set<typename decltype(graph)::element_type::vertex_descriptor>>
        exp_sets {
            { 5, 6, 7, 8 },
            { 6, 7, 14, 16, 24 },
            { 5, 8, 11, 12, 23 },
            { 11, 12, 14, 16, 23, 24 },
            { 11, 12, 14, 16, 28, 27 },
            { 11, 12, 19, 20, 28, 32 },
            { 14, 15, 16, 17, 27, 31, 33 },
            { 15, 17, 19, 20, 31, 32, 33 }
        };

    REQUIRE(cio_sets.size() == exp_sets.size());

    for (std::size_t i = 0; i < cio_sets.size(); ++i)
    {
        REQUIRE(std::all_of(cio_sets[i].begin(),
                cio_sets[i].end(),
                [&exp_sets, &i](const auto& evt) {
                    return std::any_of(exp_sets[i].begin(),
                            exp_sets[i].end(),
                            [&evt](const auto& e_evt) {
                                return evt == e_evt;
                            });
                }));
    }
}

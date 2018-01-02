#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

TEST_CASE("[trace-simple]", "Find concurrent I/O sets")
{
    static const std::string trc_file {"/home/soeste/traces/dios/24.10/trace-simple/traces.otf2"};

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = rabbitxx::find_cio_sets(*graph);

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
    auto cio_sets = rabbitxx::find_cio_sets(*graph);

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
    auto cio_sets = rabbitxx::find_cio_sets(*graph);

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
    auto cio_sets = rabbitxx::find_cio_sets(*graph);

    const std::vector<
        std::set<typename decltype(graph)::element_type::vertex_descriptor>>
        exp_sets {
            { 5, 6, 7, 8 },
            { 5, 8, 11, 12, 23 },
            { 6, 7, 14, 16, 24 },
            { 11, 12, 14, 16, 23, 24 },
            { 11, 12, 14, 16, 27, 28 },
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

TEST_CASE("[trace-own-advanced6]", "Find concurrent I/O sets")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-own_trace6_advanced/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto cio_sets = rabbitxx::find_cio_sets(*graph);
    using vertex_descriptor = typename decltype(graph)::element_type::vertex_descriptor;
    const std::vector<
        std::set<vertex_descriptor>>
        exp_sets {
            { 7, 8, 9, 10, 11, 12 },
            { 7, 8, 9, 10, 22, 23, 33, 34 },
            { 7, 8, 9, 10, 26, 27, 37 },
            { 7, 9, 11, 12, 20, 32, 35 },
            { 7, 9, 20, 22, 23, 32, 33, 34, 35 },
            { 7, 9, 20, 26, 27, 32, 35, 37 },
            { 8, 10, 11, 12, 14, 16, 28 },
            { 8, 10, 14, 16, 22, 23, 28, 33, 34 },
            { 8, 10, 14, 16, 26, 27, 28, 37 },
            { 11, 12, 14, 16, 20, 28, 32, 35 },
            { 14, 16, 20, 22, 23, 28, 32, 33, 34, 35 },
            { 14, 16, 20, 26, 27, 28, 32, 35, 37 },
            { 11, 12, 14, 16, 32, 35, 38, 43 },
            { 14, 16, 22, 23, 32, 33, 34, 35, 38, 43 },
            { 14, 16, 26, 27, 32, 35, 37, 38, 43 },
            { 11, 12, 18, 19, 32, 35, 40, 41, 43 },
            { 18, 19, 22, 23, 32, 33, 34, 35, 40, 41, 43 },
            { 18, 19, 26, 27, 32, 35, 37, 40, 41, 43 },
            { 11, 12, 18, 19, 40, 41, 45, 46, 48, 49 },
            { 18, 19, 22, 23, 33, 34, 40, 41, 45, 46, 48, 49 },
            { 18, 19, 26, 27, 37, 40, 41, 45, 46, 48, 49 },
            { 11, 12, 14, 16, 38, 45, 46, 48, 49 },
            { 14, 16, 22, 23, 33, 34, 38, 45, 46, 48, 49 },
            { 14, 16, 26, 27, 37, 38, 45, 46, 48, 49 },
        };

    REQUIRE(cio_sets.size() == exp_sets.size());

    // LOL, all things must satisfy anything! Too lazy for sorting.
    bool res = std::all_of(cio_sets.begin(), cio_sets.end(),
            // all cio-sets must statisfy ...
            [&exp_sets](const rabbitxx::set_t<vertex_descriptor>& set)
            {
                // that in any expected-set ...
                return std::any_of(exp_sets.begin(), exp_sets.end(),
                        [&set](const std::set<vertex_descriptor>& eset)
                        {
                            // all events in the cio-set satisfy ...
                            return std::all_of(set.begin(), set.end(),
                                    [&eset](const vertex_descriptor& v)
                                    {
                                        // that they are equal to some element in the expected-set.
                                        return std::any_of(eset.begin(), eset.end(),
                                            [&v](const vertex_descriptor& d) {
                                                return v == d;
                                            });
                                    });
                        });
            });

    REQUIRE(res);

}

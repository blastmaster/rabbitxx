#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

TEST_CASE("[trace-simple]", "Find correct root of synchronization events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/24.10/trace-simple/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto root_sync_evts = rabbitxx::collect_root_sync_events(*graph);
    // TODO provide a type-trait or some other more readable way to get the
    // vertex_descriptor type!
    const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts {
        0, 3, 7, 9, 17, 19, 21 };

    // Same amount?
    REQUIRE(root_sync_evts.size() == exp_evts.size());

    // each event in the root_sync_evts vector has to be in the exp_evts vector too!
    REQUIRE(std::all_of(root_sync_evts.begin(),
                      root_sync_evts.end(),
                      [&exp_evts](const auto& evt) {
                        return std::any_of(exp_evts.begin(),
                                           exp_evts.end(),
                                           [&evt](const auto& e_evt) {
                                                return e_evt == evt;
                                            });
                        }));
}


TEST_CASE("[trace-own]", "Find correct root of synchronization events")
{
    static const std::string trc_file {"/home/soeste/traces/trace-own_trace-20171116_1704_50058813361150354/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto root_sync_evts = rabbitxx::collect_root_sync_events(*graph);

    const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts {
        0, 1, 12, 17, 18, 20, 24 };

    // Same amount?
    REQUIRE(root_sync_evts.size() == exp_evts.size());

    // each event in the root_sync_evts vector has to be in the exp_evts vector too!
    REQUIRE(std::all_of(root_sync_evts.begin(),
                      root_sync_evts.end(),
                      [&exp_evts](const auto& evt) {
                        return std::any_of(exp_evts.begin(),
                                           exp_evts.end(),
                                           [&evt](const auto& e_evt) {
                                                return e_evt == evt;
                                            });
                        }));
}

TEST_CASE("[ec]", "Find correct root of synchronization events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-edgecase/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto root_sync_evts = rabbitxx::collect_root_sync_events(*graph);

    const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts {
        0, 1, 19, 21, 22, 26, 28, 32 };

    // Same amount?
    REQUIRE(root_sync_evts.size() == exp_evts.size());

    // each event in the root_sync_evts vector has to be in the exp_evts vector too!
    REQUIRE(std::all_of(root_sync_evts.begin(),
                      root_sync_evts.end(),
                      [&exp_evts](const auto& evt) {
                        return std::any_of(exp_evts.begin(),
                                           exp_evts.end(),
                                           [&evt](const auto& e_evt) {
                                                return e_evt == evt;
                                            });
                        }));
}

TEST_CASE("[ech]", "Find correct root of synchronization events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/edge_case_hard/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto root_sync_evts = rabbitxx::collect_root_sync_events(*graph);

    const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts {
        0, 1, 21, 22, 26, 29, 30, 34, 38 };

    // Same amount?
    REQUIRE(root_sync_evts.size() == exp_evts.size());

    // each event in the root_sync_evts vector has to be in the exp_evts vector too!
    REQUIRE(std::all_of(root_sync_evts.begin(),
                      root_sync_evts.end(),
                      [&exp_evts](const auto& evt) {
                        return std::any_of(exp_evts.begin(),
                                           exp_evts.end(),
                                           [&evt](const auto& e_evt) {
                                                return e_evt == evt;
                                            });
                        }));
}

TEST_CASE("[echp2p]", "check-p2p-root")
{
    static const std::string trc_file {"/home/soeste/traces/dios/edge_case_hard/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto root_sync_evts = rabbitxx::collect_root_sync_events(*graph);

    const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts {
        0, 1, 21, 22, 26, 29, 30, 34, 38 };

    const std::vector<std::pair<rabbitxx::VertexDescriptor,rabbitxx::VertexDescriptor>> p2p_roots {
        {21, 9}, {22, 10}, {26, 25}, {29, 13}, {30, 18} };

    // assert root of these root are the other
    for (const auto kvp : p2p_roots)
    {
        REQUIRE(kvp.first == rabbitxx::root_of_sync(kvp.first, *graph->get()));
        REQUIRE(kvp.first == rabbitxx::root_of_sync(kvp.second, *graph->get()));
    }

}

TEST_CASE("[trace-own-advanced6]", "Find correct root of synchronization events")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-own_trace6_advanced/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto root_sync_evts = rabbitxx::collect_root_sync_events(*graph);

    const std::vector<typename decltype(graph)::element_type::vertex_descriptor> exp_evts {
        0, 2, 25, 29, 30, 36, 39, 42, 47, 50, 56 };

    // Same amount?
    REQUIRE(root_sync_evts.size() == exp_evts.size());

    // each event in the root_sync_evts vector has to be in the exp_evts vector too!
    REQUIRE(std::all_of(root_sync_evts.begin(),
                      root_sync_evts.end(),
                      [&exp_evts](const auto& evt) {
                        return std::any_of(exp_evts.begin(),
                                           exp_evts.end(),
                                           [&evt](const auto& e_evt) {
                                                return e_evt == evt;
                                            });
                        }));
}

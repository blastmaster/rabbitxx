#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>

using rabbitxx::logging;

template<typename Set>
bool has_events(const std::vector<Set>& proc_sets, const std::vector<std::vector<int>>& exp_vals)
{
    // the number of sets should be equal
    REQUIRE(proc_sets.size() == exp_vals.size());
    for (std::size_t i = 0; i < proc_sets.size(); ++i)
    {
        bool found = std::all_of(proc_sets[i].begin(),
               proc_sets[i].end(),
                [&exp_vals, &i](const auto& evt) {
                    return std::any_of(exp_vals[i].begin(),
                            exp_vals[i].end(),
                            [&evt](const auto& e_evt) {
                                return e_evt == evt;
                            });
                    });
        if (!found) {
            return false;
        }
    }

    return true;
}

TEST_CASE("trace-simple", "[create sets per process]")
{
    // we need an absolute path here
    // otf2xx::reader should (maybe) provide std::filesystem::path overload if this
    // provides shell-expansion.
    static const std::string trc_file {"/home/soeste/traces/dios/24.10/trace-simple/traces.otf2"};

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto io_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());

    REQUIRE(!io_sets_pp->empty());
    REQUIRE(io_sets_pp->size() == 2); // size of map is 2, because we have 2 processes

    SECTION("remove empty sets and count sets per process")
    {
        rabbitxx::remove_empty_sets(*io_sets_pp);
        //each process shoud have two sets
        for (const auto& proc_sets : *io_sets_pp)
        {
            REQUIRE(proc_sets.second.size() == 2);
            // first per-process-set contains two I/O-events each.
            REQUIRE(proc_sets.second.front().size() == 2);
            // second per-process-set contains three I/O-events each.
            REQUIRE(proc_sets.second.back().size() == 3);
        }
    }
}

TEST_CASE("trace-own", "[create sets per process]")
{
    static const std::string trc_file {"/home/soeste/traces/trace-own_trace-20171116_1704_50058813361150354/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto io_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());

    REQUIRE(!io_sets_pp->empty());
    REQUIRE(io_sets_pp->size() == 4); // size of map is 4, because we have 4 processes

    rabbitxx::remove_empty_sets(*io_sets_pp);

    SECTION("remove empty sets and count sets per process")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 1:
                case 2:
                case 3:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 2);
                    break;
            }
        }
    }

    SECTION("set event ids")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {5}, {13}, {19} }));
                    break;
                case 1:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {7}, {11} }));
                    break;
                case 2:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {6}, {10} }));
                    break;
                case 3:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {14}, {16} }));
                    break;
            }
        }
    }
}

TEST_CASE("[ec]", "[create sets per process]")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-edgecase/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto io_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());

    REQUIRE(!io_sets_pp->empty());
    REQUIRE(io_sets_pp->size() == 4); // size of map is 4, because we have 4 processes

    rabbitxx::remove_empty_sets(*io_sets_pp);

    SECTION("remove empty sets and count sets per process")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 1:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 2:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 3:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
            }
        }
    }

    SECTION("set event ids")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {5}, {20}, {23, 24} }));
                    break;
                case 1:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {7}, {11}, {13, 14} }));
                    break;
                case 2:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {6}, {25}, {27} }));
                    break;
                case 3:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {8}, {15}, {17, 18} }));
                    break;
            }
        }
    }

}

TEST_CASE("[ech]", "[create sets per process]")
{
    static const std::string trc_file {"/home/soeste/traces/dios/edge_case_hard/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto io_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());

    REQUIRE(!io_sets_pp->empty());
    REQUIRE(io_sets_pp->size() == 4); // size of map is 4, because we have 4 processes

    rabbitxx::remove_empty_sets(*io_sets_pp);

    SECTION("remove empty sets and count sets per process")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 4);
                    break;
                case 1:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 2:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 4);
                    break;
                case 3:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
            }
        }
    }

    SECTION("set event ids")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {6}, {23}, {28}, {31, 33} }));
                    break;
                case 1:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {7}, {11, 12}, {15, 17} }));
                    break;
                case 2:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {5}, {24}, {27}, {32} }));
                    break;
                case 3:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {8}, {14, 16}, {19, 20} }));
                    break;
            }
        }
    }
}

TEST_CASE("[trace-own-advanced6]", "Sets per Process")
{
    static const std::string trc_file {"/home/soeste/traces/dios/rabbitxx_test/trace-own_trace6_advanced/traces.otf2"};
    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    auto io_sets_pp = rabbitxx::collect_concurrent_io_sets(*graph.get());

    REQUIRE(!io_sets_pp->empty());
    REQUIRE(io_sets_pp->size() == 6); // size of map is 6, because we have 6 processes

    rabbitxx::remove_empty_sets(*io_sets_pp);

    SECTION("remove empty sets and count sets per process")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 1:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 4);
                    break;
                case 2:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 3:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
                case 4:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 4);
                    break;
                case 5:
                    REQUIRE(io_sets_pp->operator[](proc).size() == 3);
                    break;
            }
        }
    }

    SECTION("set event ids")
    {
        for (std::size_t proc = 0; proc < io_sets_pp->size(); ++proc)
        {
            switch (proc)
            {
                case 0:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {10}, {32, 35}, {48, 49} }));
                    break;
                case 1:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {8}, {20}, {43}, {45, 46} }));
                    break;
                case 2:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {11}, {33, 34}, {37} }));
                    break;
                case 3:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {12}, {22, 23}, {26, 27} }));
                    break;
                case 4:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {9}, {28}, {38}, {40, 41} }));
                    break;
                case 5:
                    REQUIRE(has_events(io_sets_pp->operator[](proc),
                                { {7}, {14, 16}, {18, 19} }));
                    break;
            }
        }
    }

}

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <rabbitxx/cio_set.hpp>
#include <rabbitxx/graph.hpp>
#include <rabbitxx/log.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>

using rabbitxx::logging;

void dump_offset_map(const std::map<std::string, std::vector<uint64_t>>& off_map)
{
    for (const auto& file_offsets : off_map)
    {
        std::cout << "Offsets of file: " << file_offsets.first << "\n";
        for (uint64_t offset : file_offsets.second)
        {
            std::cout << offset << " ";
        }
        std::cout << "\n";
    }
}

TEST_CASE("[seek_set]", "Test offsets")
{
    static const std::string trc_file{
        "/home/soeste/code/rabbitxx/test/scorep-20190328_1750_65656537776478/traces.otf2"
    };

    // expected offsets
    static std::map<std::string, std::vector<uint64_t>> exp_offsets {
        {"/home/soeste/code/rabbitxx/test/hello1.txt", {0,0,8,0}},
        {"/home/soeste/code/rabbitxx/test/hello2.txt", {0,4096,4104,0}},
        {"/home/soeste/code/rabbitxx/test/hello3.txt", {0,8192,8200,0}},
        {"/home/soeste/code/rabbitxx/test/hello4.txt", {0,12288,12296,0}},
    };

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    const auto cio_sets = rabbitxx::find_cio_sets(graph);
    //skip mpi crap in first set, take the second
    const auto set = cio_sets[1];

    //offsets per file
    std::map<std::string, std::vector<uint64_t>> offset_map;

    for (const auto& evt : set)
    {
        const auto io_evt = rabbitxx::get_io_property(graph, evt);
        offset_map[io_evt.filename].push_back(io_evt.offset);
    }

    REQUIRE(offset_map.size() == exp_offsets.size());
    for (const auto& file_offset : offset_map)
    {
        REQUIRE(std::equal(std::begin(file_offset.second),
                    std::end(file_offset.second),
                    std::begin(exp_offsets[file_offset.first]),
                    std::end(exp_offsets[file_offset.first]))
                );
    }
}

/*
ffsets of file: /home/soeste/code/rabbitxx/test/hello1.txt
0 0 8 0
Offsets of file: /home/soeste/code/rabbitxx/test/hello2.txt
0 4096 4104 0
Offsets of file: /home/soeste/code/rabbitxx/test/hello3.txt
0 8192 8200 0
Offsets of file: /home/soeste/code/rabbitxx/test/hello4.txt
0 12288 12296 0
*/

TEST_CASE("[seek_cur]", "Test offsets")
{
    static const std::string trc_file {
        "/home/soeste/code/rabbitxx/test/scorep-20190329_2214_14646156297464/traces.otf2"
    };

    // expected offsets
    static std::map<std::string, std::vector<uint64_t>> exp_offsets {
        {"/home/soeste/code/rabbitxx/test/test_seek_cur_out.txt", {0,8,16,24,32,40,48,56,64,72,0}},
    };

    auto graph = rabbitxx::make_graph<rabbitxx::graph::OTF2_Io_Graph_Builder>(trc_file);
    const auto cio_sets = rabbitxx::find_cio_sets(graph);
    //skip mpi crap in first set, take the second
    const auto set = cio_sets[1];

    //offsets per file
    std::map<std::string, std::vector<uint64_t>> offset_map;

    for (const auto& evt : set)
    {
        const auto io_evt = rabbitxx::get_io_property(graph, evt);
        offset_map[io_evt.filename].push_back(io_evt.offset);
    }

    REQUIRE(offset_map.size() == exp_offsets.size());
    for (const auto& file_offset : offset_map)
    {
        REQUIRE(std::equal(std::begin(file_offset.second),
                    std::end(file_offset.second),
                    std::begin(exp_offsets[file_offset.first]),
                    std::end(exp_offsets[file_offset.first]))
                );
    }
}

/*
 * ffsets of file: /home/soeste/code/rabbitxx/test/test_seek_cur_out.txt
 * 0 8 16 24 32 40 48 56 64 72 0
 */

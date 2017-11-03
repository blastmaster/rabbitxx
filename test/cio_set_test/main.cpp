//#define CATCH_CONFIG_MAIN
//#include "catch.hpp"

#include <rabbitxx/graph.hpp>
#include <rabbitxx/cio_set.hpp>

template<typename Map>
void print_map(const Map& m)
{
    std::cout << "{";
    for (const auto& p : m) {
        std::cout << p.first << " : " << p.second << " ";
    }
    std::cout << "}\n";
}

//TEST_CASE("create empty cio_set", "[constructor]")
//{

//}

int main(int argc, char** argv)
{
    rabbitxx::CIO_Set<rabbitxx::otf2_trace_event, unsigned long> cs(4);
    print_map(cs.state_map());
    return 0;
}

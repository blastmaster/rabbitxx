
#include "reader.hpp"

#include <rabbitxx/log.hpp>

#include <otf2xx/otf2.hpp>

// Boost
#include <boost/mpi.hpp>

#include <iostream>

using rabbitxx::logging;
using namespace std;

int main(int argc, char** argv)
{
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    if (argc < 2) {
        env.abort(-1);
        return -1;
    }

    CallbackReader cb(world);

    string s {argv[1]};
    otf2::reader::reader reader(s);

    // get the number of location inside the trace file
    auto num_locations = reader.num_locations();

    cout << "Number of locations: " << num_locations << endl;
    cout << "Try to read all definitions ... " << endl;

    reader.set_callback(cb);

    reader.read_definitions();

    return 0;
}

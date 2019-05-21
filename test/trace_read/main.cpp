#include <iostream>
#include <string>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/callback.hpp>
#include <otf2xx/reader/reader.hpp>

std::string to_string(const otf2::common::location_group_type& type)
{
    switch (type)
    {
        case otf2::common::location_group_type::unknown:
            return {"unknown"};
        case otf2::common::location_group_type::process:
            return {"process"};
        default:
            return {"NONE"};
    }
}

inline std::ostream& operator<<(std::ostream& out, const otf2::definition::location_group& lg)
{
    out << "location_group name: " << lg.name().str()
            << " type: " << to_string(lg.type());
    return out;
}

class TestReader : public otf2::reader::callback
{

    public:
    using otf2::reader::callback::event;
    using otf2::reader::callback::definition;

    void definition(const otf2::definition::location& def) override
    {
        std::cout << def << " group: " << def.location_group() << "\n";
    }

    void definition(const otf2::definition::location_group& def) override
    {
        std::cout << "location_group name: " << def.name().str()
            << " type: " << to_string(def.type()) << "\n";
    }

};


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Error Usage: " << argv[0] << " <tracefile>\n";
        return EXIT_FAILURE;
    }

    TestReader test_rdr;
    otf2::reader::reader rdr(argv[1]);

    rdr.set_callback(test_rdr);
    rdr.read_definitions();
    rdr.read_events();

    return EXIT_SUCCESS;
}

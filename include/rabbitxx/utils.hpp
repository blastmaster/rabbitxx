#ifndef RABBITXX_UTILS_HPP
#define RABBITXX_UTILS_HPP

#include<rabbitxx/utils/enum_to_string.hpp>

#include <boost/filesystem.hpp>

namespace rabbitxx {

namespace fs = boost::filesystem;

template<typename DurationT=otf2::chrono::microseconds>
std::string duration_to_string(const otf2::chrono::duration& dur)
{
    std::stringstream ss;
    // need .count() ?
    ss << std::chrono::duration_cast<DurationT>(dur);
    return ss.str();
}

}

#endif // RABBITXX_UTILS_HPP

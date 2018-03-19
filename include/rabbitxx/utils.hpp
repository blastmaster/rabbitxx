#ifndef RABBITXX_UTILS_HPP
#define RABBITXX_UTILS_HPP

#include <boost/filesystem.hpp>

namespace rabbitxx {

namespace fs = boost::filesystem;

template<typename DurationT=otf2::chrono::nanoseconds>
std::string duration_to_string(const otf2::chrono::duration& dur)
{
    std::stringstream ss;
    ss << otf2::chrono::duration_cast<DurationT>(dur);
    return ss.str();
}

}

#endif // RABBITXX_UTILS_HPP

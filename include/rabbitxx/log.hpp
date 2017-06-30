#ifndef __RABBITXX_LOG_HPP__
#define __RABBITXX_LOG_HPP__

#include <nitro/log/log.hpp>
#include <nitro/log/sink/stdout.hpp>

#include <nitro/log/attribute/message.hpp>
#include <nitro/log/attribute/hostname.hpp>
#include <nitro/log/attribute/timestamp.hpp>
#include <nitro/log/attribute/severity.hpp>
#include <nitro/log/attribute/mpi_rank.hpp>

#include <nitro/log/filter/and_filter.hpp>
#include <nitro/log/filter/mpi_master_filter.hpp>
#include <nitro/log/filter/severity_filter.hpp>

namespace rabbitxx
{
    namespace log
    {
        namespace detail
        {
            typedef nitro::log::record<nitro::log::message_attribute,
                                       nitro::log::timestamp_attribute,
                                       nitro::log::hostname_attribute,
                                       nitro::log::severity_attribute,
                                       nitro::log::mpi_rank_attribute> record;

            template<typename Record>
            class rabbitxx_log_formater
            {
                public:
                    std::string format(Record& r)
                    {
                        std::stringstream msg;

                        msg << "[" << r.timestamp().count() << "][Rank: " << r.mpi_rank()
                            << "][" << r.severity() << "]: " << r.message();

                        return msg.str();
                    }
            };

            template<typename Record>
            using rabbitxx_log_filter = nitro::log::filter::severity_filter<Record>;

        } // namespace detail

        typedef nitro::log::logger<detail::record, detail::rabbitxx_log_formater,
                                   nitro::log::sink::stdout, detail::rabbitxx_log_filter> logging;

        inline void set_min_severity_level(nitro::log::severity_level sev)
        {
            detail::rabbitxx_log_filter<detail::record>::set_severity(sev);
        }

        inline void set_min_severity_level(std::string verbosity)
        {
            set_min_severity_level(
                    nitro::log::severity_from_string(verbosity, nitro::log::severity_level::debug));
        }

    } // namespace log

    using log::logging;

} // namespace rabbitxx

#endif // __RABBITXX_LOG_HPP__

#ifndef __READER_HPP__
#define __READER_HPP__

#include <rabbitxx/trace/base.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/reader.hpp>

#include <rabbitxx/log.hpp>

class CallbackReader : public rabbitxx::trace::base
{
    typedef rabbitxx::trace::base base;

    public:
        using otf2::reader::callback::event;
        using otf2::reader::callback::definition;

        CallbackReader(boost::mpi::communicator& comm) : base(comm)
        {
        }

        ~CallbackReader() = default;

        virtual void definition(const otf2::definition::io_paradigm& definition) override
        {
            rabbitxx::logging::debug() << "Found io_paradigm defintion";
        }

        virtual void definition(const otf2::definition::io_file& definition) override
        {
            rabbitxx::logging::debug() << "Found io_file defintion";
        }

        virtual void definition(const otf2::definition::io_handle& definition) override
        {
            rabbitxx::logging::debug() << "Found io_handle defintion";
        }

        virtual void definition(const otf2::definition::io_file_property& definition) override
        {
            rabbitxx::logging::debug() << "Found io_file_property defintion";
        }

        virtual void definition(const otf2::definition::unknown& definition) override
        {
            rabbitxx::logging::warn() << "Found unknown defintion";
        }

        virtual void definitions_done(const otf2::reader::reader& rdr) override
        {
        }

};

#endif // __READER_HPP__

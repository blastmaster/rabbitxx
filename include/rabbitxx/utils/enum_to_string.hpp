#ifndef __ENUM_TO_STRING_HPP__
#define __ENUM_TO_STRING_HPP__

#include <otf2xx/common.hpp>

#include <string>


// helper function, return string representation for io_operation_mode
inline
std::string to_string(const otf2::common::io_operation_mode_type& mode) noexcept
{
    switch (mode)
    {
        case otf2::common::io_operation_mode_type::read:
            return {"read"};
        case otf2::common::io_operation_mode_type::write:
            return {"write"};
        case otf2::common::io_operation_mode_type::flush:
            return {"flush"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::io_operation_flag_type& flag) noexcept
{
    switch (flag)
    {
        case otf2::common::io_operation_flag_type::none:
            return {"none"};
        case otf2::common::io_operation_flag_type::non_blocking:
            return {"non blocking"};
        case otf2::common::io_operation_flag_type::collective:
            return {"collective"};
        default:
            return {"UNDEFINED"};
    }
}


// helper function, return string representation for io_seek_option_type
inline
std::string to_string(const otf2::common::io_seek_option_type& op_type) noexcept
{
    switch (op_type)
    {
        case otf2::common::io_seek_option_type::from_start:
            return {"start"};
        case otf2::common::io_seek_option_type::from_current:
            return {"current"};
        case otf2::common::io_seek_option_type::from_end:
            return {"end"};
        case otf2::common::io_seek_option_type::data:
            return {"data"};
        case otf2::common::io_seek_option_type::hole:
            return {"hole"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::io_access_mode_type& acc_mode) noexcept
{
    switch (acc_mode)
    {
        case otf2::common::io_access_mode_type::read_only:
            return {"read only"};
        case otf2::common::io_access_mode_type::write_only:
            return {"write only"};
        case otf2::common::io_access_mode_type::read_write:
            return {"read/write"};
        case otf2::common::io_access_mode_type::execute_only:
            return {"execute only"};
        case otf2::common::io_access_mode_type::search_only:
            return {"search only"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::io_creation_flag_type& creation_flag) noexcept
{
    switch (creation_flag)
    {
        case otf2::common::io_creation_flag_type::none:
            return {"none"};
        case otf2::common::io_creation_flag_type::create:
            return {"create"};
        case otf2::common::io_creation_flag_type::truncate:
            return {"truncate"};
        case otf2::common::io_creation_flag_type::directory:
            return {"directory"};
        case otf2::common::io_creation_flag_type::exclusive:
            return {"exclusive"};
        case otf2::common::io_creation_flag_type::no_controlling_terminal:
            return {"no controlling terminal"};
        case otf2::common::io_creation_flag_type::no_follow:
            return {"no follow"};
        case otf2::common::io_creation_flag_type::path:
            return {"path"};
        case otf2::common::io_creation_flag_type::temporary_file:
            return {"temporary file"};
        case otf2::common::io_creation_flag_type::largefile:
            return {"large file"};
        case otf2::common::io_creation_flag_type::no_seek:
            return {"no seek"};
        case otf2::common::io_creation_flag_type::unique:
            return {"unique"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::io_status_flag_type& status_flag) noexcept
{
    switch (status_flag)
    {
        case otf2::common::io_status_flag_type::none:
            return {"none"};
        case otf2::common::io_status_flag_type::close_on_exec:
            return {"close on exec"};
        case otf2::common::io_status_flag_type::append:
            return {"append"};
        case otf2::common::io_status_flag_type::non_blocking:
            return {"non blocking"};
        case otf2::common::io_status_flag_type::async:
            return {"async"};
        case otf2::common::io_status_flag_type::sync:
            return {"sync"};
        case otf2::common::io_status_flag_type::data_sync:
            return {"data sync"};
        case otf2::common::io_status_flag_type::avoid_caching:
            return {"avoid caching"};
        case otf2::common::io_status_flag_type::no_access_time:
            return {"no access time"};
        case otf2::common::io_status_flag_type::delete_on_close:
            return {"delete on close"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::collective_type& coll_type) noexcept
{
    switch (coll_type)
    {
        case otf2::common::collective_type::barrier:
            return {"barrier"};
        case otf2::common::collective_type::broadcast:
            return {"broadcast"};
        case otf2::common::collective_type::gather:
            return {"gather"};
        case otf2::common::collective_type::gatherv:
            return {"gatherv"};
        case otf2::common::collective_type::scatter:
            return {"scatter"};
        case otf2::common::collective_type::scatterv:
            return {"scatterv"};
        case otf2::common::collective_type::all_gather:
            return {"all gather"};
        case otf2::common::collective_type::all_gatherv:
            return {"all gatherv"};
        case otf2::common::collective_type::all_to_all:
            return {"all to all"};
        case otf2::common::collective_type::all_to_allv:
            return {"all to allv"};
        case otf2::common::collective_type::all_to_allw:
            return {"all to allw"};
        case otf2::common::collective_type::all_reduce:
            return {"all reduce"};
        case otf2::common::collective_type::reduce_scatter:
            return {"reduce scatter"};
        case otf2::common::collective_type::scan:
            return {"scan"};
        case otf2::common::collective_type::exscan:
            return {"exscan"};
        case otf2::common::collective_type::reduce_scatter_block:
            return {"reduce scatter block"};
        case otf2::common::collective_type::create_handle:
            return {"create handle"};
        case otf2::common::collective_type::destroy_handle:
            return {"destroy handle"};
        case otf2::common::collective_type::allocate:
            return {"allocate"};
        case otf2::common::collective_type::deallocate:
            return {"deallocate"};
        case otf2::common::collective_type::create_handle_and_allocate:
            return {"create handle and allocate"};
        case otf2::common::collective_type::destroy_handle_and_deallocate:
            return {"destroy handle and deallocate"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::group_type& grp_type) noexcept
{
    switch (grp_type)
    {
        case otf2::common::group_type::unknown:
            return {"unknown"};
        case otf2::common::group_type::locations:
            return {"locations"};
        case otf2::common::group_type::regions:
            return {"regions"};
        case otf2::common::group_type::metric:
            return {"metric"};
        case otf2::common::group_type::comm_locations:
            return {"comm locations"};
        case otf2::common::group_type::comm_group:
            return {"comm group"};
        case otf2::common::group_type::comm_self:
            return {"comm self"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::group_flag_type& grp_flag_type) noexcept
{
    switch (grp_flag_type)
    {
        case otf2::common::group_flag_type::none:
            return {"none"};
        case otf2::common::group_flag_type::global_members:
            return {"global members"};
        default:
            return {"UNDEFINED"};
    }
}

inline
std::string to_string(const otf2::common::paradigm_type& para_type) noexcept
{
    switch (para_type)
    {
        case otf2::common::paradigm_type::unknown:
            return {"unknown"};
        case otf2::common::paradigm_type::user:
            return {"user"};
        case otf2::common::paradigm_type::compiler:
            return {"compiler"};
        case otf2::common::paradigm_type::openmp:
            return {"openmp"};
        case otf2::common::paradigm_type::mpi:
            return {"mpi"};
        case otf2::common::paradigm_type::cuda:
            return {"cuda"};
        case otf2::common::paradigm_type::measurement_system:
            return {"measurement system"};
        case otf2::common::paradigm_type::pthread:
            return {"pthread"};
        case otf2::common::paradigm_type::hmpp:
            return {"hmpp"};
        case otf2::common::paradigm_type::ompss:
            return {"ompss"};
        case otf2::common::paradigm_type::hardware:
            return {"hardware"};
        case otf2::common::paradigm_type::gaspi:
            return {"gaspi"};
        case otf2::common::paradigm_type::upc:
            return {"upc"};
        case otf2::common::paradigm_type::shmem:
            return {"shmem"};
        case otf2::common::paradigm_type::winthread:
            return {"winthread"};
        case otf2::common::paradigm_type::qtthread:
            return {"qtthread"};
        case otf2::common::paradigm_type::acethread:
            return {"acethread"};
        case otf2::common::paradigm_type::tbbthread:
            return {"tbbthread"};
        case otf2::common::paradigm_type::openacc:
            return {"openacc"};
        case otf2::common::paradigm_type::opencl:
            return {"opencl"};
        case otf2::common::paradigm_type::mtapi:
            return {"mtapi"};
        case otf2::common::paradigm_type::sampling:
            return {"sampling"};
        case otf2::common::paradigm_type::none:
            return {"none"};
        default:
            return {"UNDEFINED"};
    }
}

#endif /* __ENUM_TO_STRING_HPP__ */

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "rabbitxx")
set(PROJECT_VERSION "0.0.1")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

#Initialize git submodules if not done already
include(cmake/GitSubmoduleUpdate.cmake)
git_submodule_update()

include(CTest)

option(BUILD_TESTS "build tests" ON)
option(BUILD_MODULES "build modules" ON)
option(BUILD_DEBUG "enable debug mode" ON)

add_compile_options(-Wall -pedantic -Wextra -std=c++14)

if(${BUILD_DEBUG})
    add_compile_options(-pg -ggdb -O0)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

find_package(Boost 1.62 REQUIRED COMPONENTS program_options system filesystem mpi graph)
find_package(MPI REQUIRED)

include_directories(SYSTEM ${MPI_INCLUDE_PATH})

include(ExternalProject)
ExternalProject_Add(rapidjson-project
    PREFIX "${PROJECT_SOURCE_DIR}/external/rapidjson-test"
    GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
    GIT_PROGRESS 1
    GIT_TAG f54b0e47a08782a6131cc3d60f94d038fa6e0a51
    CMAKE_ARGS
        -DRAPIDJSON_BUILD_EXAMPLES=OFF
        -DRAPIDJSON_BUILD_TESTS=OFF
        -DRAPIDJSON_BUILD_DOC=OFF
    INSTALL_COMMAND ""
)

set(RapidJSON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/external/rapidjson-test/src/rapidjson-project/include/ CACHE PATH "rapidjson include dir" FORCE)
message("check for rapidjson target")
if(NOT TARGET RapidJSON::RapidJSON)
    message("rapidjson target not found")
    add_library(RapidJSON::RapidJSON INTERFACE IMPORTED)
    set_target_properties(RapidJSON::RapidJSON PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RapidJSON_INCLUDE_DIR}"
    )
    message("RapidJSON_INCLUDE_DIR: ${RapidJSON_INCLUDE_DIR}")
endif()

add_subdirectory(external/nitro)
add_subdirectory(external/otf2xx)
mark_as_advanced(OTF2XX_CHRONO_DURATION_TYPE OTF2XX_WITH_MPI OTF2_CONFIG OTF2_PRINT)

set(RABBITXX_SOURCES
    ${CMAKE_SOURCE_DIR}/src/cio_set.cpp
    ${CMAKE_SOURCE_DIR}/src/io_graph.cpp
    ${CMAKE_SOURCE_DIR}/src/otf2_io_graph_builder.cpp
    ${CMAKE_SOURCE_DIR}/src/stats.cpp
    ${CMAKE_SOURCE_DIR}/src/experiment.cpp
)

add_library(rabbitxx-core ${RABBITXX_SOURCES})
target_include_directories(rabbitxx-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rabbitxx-core
    PUBLIC
        Nitro::nitro-log
        otf2xx::Reader
        Boost::system
        Boost::graph
        Boost::filesystem
        Boost::mpi
        MPI::MPI_CXX
)

add_library(rabbitxx::core ALIAS rabbitxx-core)

if (BUILD_MODULES)
    add_subdirectory(modules)
endif()

if (BUILD_TESTS)
    set(CATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/catch/single_include CACHE INTERNAL "Path to include folder for Catch")
    include_directories(${CATCH_INCLUDE_DIR})
    enable_testing()
    add_subdirectory(test)
endif()

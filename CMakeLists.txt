cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "rabbitxx")
set(PROJECT_VERSION "0.0.1")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

include(CTest)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

option(BUILD_TESTS "build tests" ON)
option(BUILD_MODULES "build modules" ON)
option(BUILD_DEBUG "enable debug mode" ON)

set(RABBITXX_CXX_FLAGS "-Wall -pedantic -Wextra -std=c++14")
set(RABBITXX_C_FLAGS "-Wall -pedantic -Wextra")

if(${BUILD_DEBUG})
    set(RABBITXX_CXX_FLAGS "${RABBITXX_CXX_FLAGS} -pg -ggdb -O0")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(RABBITXX_C_FLAGS "${RABBITXX_C_FLAGS} -pg -ggdb -O0")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Boost 1.62 REQUIRED COMPONENTS program_options system filesystem mpi graph)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

file(GLOB SUBMODULE_FILES "external/nitro/*")
list(LENGTH SUBMODULE_FILES COUNT_NITRO)
file(GLOB SUBMODULE_FILES "external/otf2xx/*")
list(LENGTH SUBMODULE_FILES COUNT_OTF2XX)
file(GLOB SUBMODULE_FILES "external/catch/*")
list(LENGTH SUBMODULE_FILES COUNT_CATCH)
if(${COUNT_NITRO} EQUAL 0 OR ${COUNT_OTF2XX} EQUAL 0 OR ${COUNT_CATCH} EQUAL 0)
    #TODO: using wrong branch of otf2xx
    message(STATUS "Initializing submodules")
    execute_process(COMMAND "git submodule update --init --recursive" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

include(external/nitro/Nitro.cmake)

set(OTF2XX_WITH_MPI ON CACHE BOOL "Whether OTF2xx should be build with MPI support or not (Requires Boost.MPI)" FORCE)
include(external/otf2xx/OTF2XX.cmake)
set(OTF2XX_CHRONO_DURATION_TYPE nanoseconds)
#add_subdirectory(external/otf2xx)
set(OTF2XX_WITH_MPI ON CACHE BOOL "Whether OTF2xx should be build with MPI support or not (Requires Boost.MPI)" FORCE)

include_directories("${CMAKE_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RABBITXX_CXX_FLAGS}")

set(RABBITXX_SOURCES
    ${CMAKE_SOURCE_DIR}/src/cio_set.cpp
    ${CMAKE_SOURCE_DIR}/src/io_graph.cpp
    ${CMAKE_SOURCE_DIR}/src/otf2_io_graph_builder.cpp
)

#TODO: build individual static libraries
add_library(rabbitxx_core ${RABBITXX_SOURCES})
set(RABBITXX_LIBRARIES rabbitxx_core)
#set(RABBITXX_LIBRARIES rabbitxx_core otf2xx::Reader)
#set(RABBITXX_LIBRARIES rabbitxx_core otf2xx-core otf2xx-reader)

if (BUILD_MODULES)
    add_subdirectory(modules)
endif()

if (BUILD_TESTS)
    set(CATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/catch/single_include CACHE INTERNAL "Path to include folder for Catch")
    include_directories(${CATCH_INCLUDE_DIR})
    enable_testing()
    add_subdirectory(test)
endif()

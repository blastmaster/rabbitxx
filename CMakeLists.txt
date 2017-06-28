cmake_minimum_required(VERSION 3.2.2)

set(PROJECT_NAME "rabbitxx")
set(PROJECT_VERSION "0.0.1")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(RABBITXX_CXX_FLAGS "-Wall -pedantic -Wextra -std=c++14")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Boost 1.62 REQUIRED COMPONENTS program_options system filesystem graph_parallel)

message(STATUS "Boost Include dir: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

file(GLOB SUBMODULE_FILES "external/nitro/*")
list(LENGTH SUBMODULE_FILES COUNT_NITRO)
file(GLOB SUBMODULE_FILES "external/otf2xx/*")
list(LENGTH SUBMODULE_FILES COUNT_OTF2XX)
file(GLOB SUBMODULE_FILES "external/Catach/*")
list(LENGTH SUBMODULE_FILES COUNT_CATCH)
if(${COUNT_NITRO} EQUAL 0 OR ${COUNT_OTF2XX} EQUAL 0 OR ${COUNT_CATCH} EQUAL 0)
    #TODO: using wrong branch of otf2xx
    message(STATUS "Initializing submodules")
    execute_process(COMMAND "git submodule update --init --recursive" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

include(external/nitro/Nitro.cmake)

include_directories("${CMAKE_SOURCE_DIR}/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RABBITXX_CXX_FLAGS}")

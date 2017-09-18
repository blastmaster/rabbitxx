set(SOURCE
    main.cpp
)

add_executable(count_io_events ${SOURCE})
target_link_libraries(count_io_events
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

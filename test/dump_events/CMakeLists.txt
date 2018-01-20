set(SOURCE
    main.cpp
)

add_executable(dump_events ${SOURCE})
target_link_libraries(dump_events
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

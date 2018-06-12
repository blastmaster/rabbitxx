set(SOURCE
    main.cpp
)

add_executable(graph_properties_test ${SOURCE})
target_link_libraries(graph_properties_test
    ${RABBITXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)

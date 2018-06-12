set(SOURCE
    main.cpp
)

add_executable(simple_graph_test ${SOURCE})
target_link_libraries(simple_graph_test
    ${RABBITXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)
add_test(NAME simple_graph_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/simple_graph_test)

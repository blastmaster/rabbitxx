set(SOURCE
    main.cpp
)

add_executable(simple_graph_test ${SOURCE})
target_link_libraries(simple_graph_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

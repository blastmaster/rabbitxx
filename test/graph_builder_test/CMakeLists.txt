set(SOURCE
    main.cpp
)

add_executable(graph_builder_test ${SOURCE})
target_link_libraries(graph_builder_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

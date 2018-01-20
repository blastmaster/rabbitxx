set(SOURCE
    main.cpp
)

add_executable(out_degree_test ${SOURCE})
target_link_libraries(out_degree_test
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

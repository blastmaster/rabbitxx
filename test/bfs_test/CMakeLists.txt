set(SOURCE
    main.cpp
)

add_executable(bfs_test ${SOURCE})
target_link_libraries(bfs_test
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

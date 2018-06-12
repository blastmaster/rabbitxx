set(SOURCE
    main.cpp
)

add_executable(cio_set_test ${SOURCE})
target_link_libraries(cio_set_test
    ${RABBITXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)

set(SOURCE
    main.cpp
)

add_executable(read_test ${SOURCE})
target_link_libraries(read_test
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)

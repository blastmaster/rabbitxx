set(SOURCE
    main.cpp
)

add_executable(read_test ${SOURCE})
target_link_libraries(read_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

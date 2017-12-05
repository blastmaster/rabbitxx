set(SOURCE
    main.cpp
)

add_executable(independent_process_group_test ${SOURCE})
target_link_libraries(independent_process_group_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

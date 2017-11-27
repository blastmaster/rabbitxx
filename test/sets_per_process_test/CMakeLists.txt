set(SOURCE
    main.cpp
)

add_executable(sets_per_process_test ${SOURCE})
target_link_libraries(sets_per_process_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

set(SOURCE
    main.cpp
)

add_executable(dump_sets_per_process ${SOURCE})
target_link_libraries(dump_sets_per_process
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

set(SOURCE
    main.cpp
)

add_executable(sync_root_test ${SOURCE})
target_link_libraries(sync_root_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

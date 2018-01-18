set(SOURCE
    ${RABBITXX_SOURCES}
    main.cpp
)

add_executable(sets_to_dot_test ${SOURCE})
target_link_libraries(sets_to_dot_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

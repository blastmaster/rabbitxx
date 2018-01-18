set(SOURCE
    ${RABBITXX_SOURCES}
    main.cpp
)

add_executable(dot_output_test ${SOURCE})
target_link_libraries(dot_output_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

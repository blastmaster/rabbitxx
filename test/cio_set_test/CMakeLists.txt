set(SOURCE
    ${RABBITXX_SOURCES}
    main.cpp
)

add_executable(cio_set_test ${SOURCE})
target_link_libraries(cio_set_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

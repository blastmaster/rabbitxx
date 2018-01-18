set(SOURCE
    ${RABBITXX_SOURCES}
    main.cpp
)

add_executable(mapping_test ${SOURCE})
target_link_libraries(mapping_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

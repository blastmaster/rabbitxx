set(SOURCES 
    main.cpp
)

add_executable(concurrent-io-sets ${SOURCES})
target_link_libraries(concurrent-io-sets
    ${RABBITXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)

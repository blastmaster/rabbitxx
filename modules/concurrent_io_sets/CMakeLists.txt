set(SOURCES 
    ${RABBITXX_SOURCES}
    main.cpp
)

add_executable(concurrent-io-sets ${SOURCES})
target_link_libraries(concurrent-io-sets
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

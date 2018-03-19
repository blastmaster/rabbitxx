set(SOURCES 
    main.cpp
)

add_executable(request_sizes ${SOURCES})
target_link_libraries(request_sizes
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

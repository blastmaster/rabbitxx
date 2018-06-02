set(SOURCES 
    main.cpp
)

add_executable(num_vertices ${SOURCES})
target_link_libraries(num_vertices
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

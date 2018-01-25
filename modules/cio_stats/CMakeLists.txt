set(SOURCES 
    main.cpp
)

add_executable(cio_stats ${SOURCES})
target_link_libraries(cio_stats
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

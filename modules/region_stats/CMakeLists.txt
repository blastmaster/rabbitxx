set(SOURCES 
    main.cpp
)

add_executable(region_stats ${SOURCES})
target_link_libraries(region_stats
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

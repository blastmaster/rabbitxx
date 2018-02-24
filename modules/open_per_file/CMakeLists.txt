set(SOURCES 
    main.cpp
)

add_executable(open-per-file ${SOURCES})
target_link_libraries(open-per-file
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

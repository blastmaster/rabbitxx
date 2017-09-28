set(SOURCES 
    main.cpp
)

add_executable(print-graph ${SOURCES})
target_link_libraries(print-graph
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

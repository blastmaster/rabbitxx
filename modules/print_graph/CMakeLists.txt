set(SOURCES 
    main.cpp
)

add_executable(print-graph ${SOURCES})
file(COPY trc2png.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(print-graph
    ${RABBITXX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)

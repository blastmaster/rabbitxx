set(SOURCE
    ${RABBITXX_SOURCES}
    main.cpp
)

add_executable(process_group_test ${SOURCE})
target_link_libraries(process_group_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)
add_test(NAME process_group_test  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/process_group_test)

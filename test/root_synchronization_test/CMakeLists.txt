set(SOURCE
    main.cpp
)

add_executable(root_synchronization_test ${SOURCE})
target_link_libraries(root_synchronization_test
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)
add_test(NAME root_synchronization_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/root_synchronization_test)

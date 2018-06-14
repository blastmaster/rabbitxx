set(SOURCE
    main.cpp
)

add_executable(root_synchronization_test ${SOURCE})
target_link_libraries(root_synchronization_test
    PRIVATE
    rabbitxx::core
)
add_test(NAME root_synchronization_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/root_synchronization_test)

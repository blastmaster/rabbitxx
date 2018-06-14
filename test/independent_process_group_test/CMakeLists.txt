set(SOURCE
    main.cpp
)

add_executable(independent_process_group_test ${SOURCE})
target_link_libraries(independent_process_group_test
    PRIVATE
    rabbitxx::core
)
add_test(NAME independent_process_group_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/independent_process_group_test)

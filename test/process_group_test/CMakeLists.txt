set(SOURCE
    main.cpp
)

add_executable(process_group_test ${SOURCE})
target_link_libraries(process_group_test
    PRIVATE
    rabbitxx::core
)
add_test(NAME process_group_test  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/process_group_test)

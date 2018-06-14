set(SOURCE
    main.cpp
)

add_executable(concurrent_io_sets_test ${SOURCE})
target_link_libraries(concurrent_io_sets_test
    PRIVATE
    rabbitxx::core
)
add_test(NAME concurrent_io_sets_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/concurrent_io_sets_test)

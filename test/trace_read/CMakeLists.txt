set(SOURCE
    main.cpp
)

add_executable(trace_read ${SOURCE})
target_link_libraries(trace_read
    PRIVATE
    rabbitxx::core
)
add_test(NAME trace_read  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/trace_read)

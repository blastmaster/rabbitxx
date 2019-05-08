set(SOURCE
    main.cpp
)

add_executable(trace_read ${SOURCE})
target_link_libraries(trace_read
    PRIVATE
    rabbitxx::core
)

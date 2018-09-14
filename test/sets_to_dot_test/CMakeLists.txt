set(SOURCE
    main.cpp
)

add_executable(sets_to_dot_test ${SOURCE})
target_link_libraries(sets_to_dot_test
    PRIVATE
    rabbitxx::core
)

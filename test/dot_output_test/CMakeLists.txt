set(SOURCE
    main.cpp
)

add_executable(dot_output_test ${SOURCE})
target_link_libraries(dot_output_test
    PRIVATE
    rabbitxx::core
)

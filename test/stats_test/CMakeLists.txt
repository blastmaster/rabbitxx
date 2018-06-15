set(SOURCE
    main.cpp
)

add_executable(stats_test ${SOURCE})
target_link_libraries(stats_test
    PRIVATE
    rabbitxx::core
)

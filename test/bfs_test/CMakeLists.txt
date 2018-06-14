set(SOURCE
    main.cpp
)

add_executable(bfs_test ${SOURCE})
target_link_libraries(bfs_test
    PRIVATE
    rabbitxx::core
)

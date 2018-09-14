set(SOURCE
    main.cpp
)

add_executable(dfs_test ${SOURCE})
target_link_libraries(dfs_test
    PRIVATE
    rabbitxx::core
)

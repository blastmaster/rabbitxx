set(SOURCE
    main.cpp
)

add_executable(graph_properties_test ${SOURCE})
target_link_libraries(graph_properties_test
    PRIVATE
    rabbitxx::core
)

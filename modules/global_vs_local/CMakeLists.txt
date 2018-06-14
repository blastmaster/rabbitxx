set(SOURCES 
    main.cpp
)

add_executable(global_vs_local ${SOURCES})
target_link_libraries(global_vs_local
    PRIVATE
    rabbitxx::core
)

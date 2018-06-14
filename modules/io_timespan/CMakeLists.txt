set(SOURCES 
    main.cpp
)

add_executable(io_timespan ${SOURCES})
target_link_libraries(io_timespan
    PRIVATE
    rabbitxx::core
)

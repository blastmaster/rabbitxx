set(SOURCES 
    main.cpp
)

add_executable(io_timespan ${SOURCES})
target_link_libraries(io_timespan
    ${RABBITXX_LIBRARIES}
)

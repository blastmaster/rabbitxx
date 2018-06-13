set(SOURCES 
    main.cpp
)

add_executable(ops_per_file ${SOURCES})
target_link_libraries(ops_per_file
    ${RABBITXX_LIBRARIES}
)

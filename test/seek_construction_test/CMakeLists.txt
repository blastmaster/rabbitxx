set(SOURCE
    main.cpp
)

add_executable(seek_construction_test ${SOURCE})
target_link_libraries(seek_construction_test
    PRIVATE
    rabbitxx::core
)
add_test(NAME seek_construction_test COMMAND
    ${CMAKE_CURRENT_BINARY_DIR}/seek_construction_test)

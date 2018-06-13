set(SOURCE
    main.cpp
)

add_executable(sets_per_process_test ${SOURCE})
target_link_libraries(sets_per_process_test
    ${RABBITXX_LIBRARIES}
)
add_test(NAME sets_per_process_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sets_per_process_test)

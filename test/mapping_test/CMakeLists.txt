set(SOURCE
    main.cpp
)

add_executable(mapping_test ${SOURCE})
target_link_libraries(mapping_test
    ${RABBITXX_LIBRARIES}
)

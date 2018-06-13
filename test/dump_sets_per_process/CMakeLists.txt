set(SOURCE
    main.cpp
)

add_executable(dump_sets_per_process ${SOURCE})
target_link_libraries(dump_sets_per_process
    ${RABBITXX_LIBRARIES}
)

set(SOURCES 
    main.cpp
)

add_executable(significant_op_type ${SOURCES})
target_link_libraries(significant_op_type
    ${RABBITXX_LIBRARIES}
)

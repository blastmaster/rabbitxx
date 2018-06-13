set(SOURCES 
    main.cpp
)

add_executable(creates-in-dir ${SOURCES})
target_link_libraries(creates-in-dir
    ${RABBITXX_LIBRARIES}
)

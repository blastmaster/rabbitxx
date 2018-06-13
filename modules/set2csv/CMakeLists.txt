set(SOURCES 
    main.cpp
)

add_executable(set2csv ${SOURCES})
target_link_libraries(set2csv
    ${RABBITXX_LIBRARIES}
    Boost::program_options
)

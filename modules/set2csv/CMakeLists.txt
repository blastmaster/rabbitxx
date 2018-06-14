set(SOURCES 
    main.cpp
)

add_executable(set2csv ${SOURCES})
target_link_libraries(set2csv
    PRIVATE
    rabbitxx::core
    Boost::program_options
)

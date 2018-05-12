set(SOURCES 
    main.cpp
)

add_executable(set2csv ${SOURCES})
target_link_libraries(set2csv
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

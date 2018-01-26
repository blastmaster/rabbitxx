set(SOURCES 
    main.cpp
)

add_executable(creates-in-dir ${SOURCES})
target_link_libraries(creates-in-dir
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

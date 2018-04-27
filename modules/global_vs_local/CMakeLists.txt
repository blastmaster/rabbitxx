set(SOURCES 
    main.cpp
)

add_executable(global_vs_local ${SOURCES})
target_link_libraries(global_vs_local
    ${RABBITXX_LIBRARIES}
    ${OTF2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    otf2xx
)

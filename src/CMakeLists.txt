
add_executable(rabbitxx main.cpp)

# link against otf2xx
target_link_libraries(rabbitxx otf2xx)
# link against boost stuff
target_link_libraries(rabbitxx ${OTF2_LIBRARIES} ${Boost_LIBRARIES})
# link against mpi stuff
target_link_libraries(rabbitxx ${MPI_CXX_LIBRARIES})

install(TARGETS rabbitxx DESTINATION bin)

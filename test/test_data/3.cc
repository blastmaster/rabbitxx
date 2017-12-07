#include <mpi.h>
#include <iostream>

using namespace std;
int main(int argc, char *argv[])
{
    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if(world_size < 4){
        std::cout << "This Programm is designed for 4 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);

    for (int i = 0; i < 2; ++i) {
        int number = 1;

        switch(world_rank){
            case 0: MPI_Recv(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    break;
            case 1: MPI_Send(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
            case 2: MPI_Recv(&number, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    break;
            case 3: MPI_Send(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
                    break;
            default: std::cout << "World Rank not found, use -n 4" << std::endl;
                    break;
        }

        if(i == 0){
            fflush(stdout);
            switch(world_rank){
                case 0: MPI_Recv(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        break;
                case 2: MPI_Send(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
            }
        }

        fflush(stdout);
    }


    if(world_rank != 2){ fflush(stdout); }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}

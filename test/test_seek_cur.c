#include <mpi.h>
#include <stdio.h>

static const long BLKSZ = 4096L;
static const char* testf1 = "test_seek_cur_out.txt";

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE *f1 = NULL;
    char *buf = "fooooo";
    int ret = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        f1 = fopen(testf1, "w+");

        ret = fwrite((void*) buf, sizeof(buf), 1, f1);
        fseek(f1, ret*sizeof(buf), SEEK_CUR);
        ret = fwrite((void*) buf, sizeof(buf), 1, f1);
        fseek(f1, ret*sizeof(buf), SEEK_CUR);
        ret = fwrite((void*) buf, sizeof(buf), 1, f1);
        fseek(f1, ret*sizeof(buf), SEEK_CUR);
        ret = fwrite((void*) buf, sizeof(buf), 1, f1);
        fseek(f1, ret*sizeof(buf), SEEK_CUR);
        ret = fwrite((void*) buf, sizeof(buf), 1, f1);

        fclose(f1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

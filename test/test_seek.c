#include <mpi.h>
#include <stdio.h>


// scorep-20180627_1302_24566201227264

static const long BLKSZ = 4096L;

static const char* testf1 = "hello1.txt";
static const char* testf2 = "hello2.txt";
static const char* testf3 = "hello3.txt";
static const char* testf4 = "hello4.txt";


int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE *f1 = NULL;
    FILE *f2 = NULL;
    FILE *f3 = NULL;
    FILE *f4 = NULL;
    char *buf = "fooooo";
    int ret = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        f1 = fopen(testf1, "w+");
        f2 = fopen(testf2, "w+");
        f3 = fopen(testf3, "w+");
        f4 = fopen(testf4, "w+");

        fseek(f1, 0, SEEK_SET);
        ret = fwrite((void*) buf, sizeof(buf), 1, f1);

        fseek(f2, BLKSZ, SEEK_SET);
        ret = fwrite((void*) buf, sizeof(buf), 1, f2);

        fseek(f3, BLKSZ*2, SEEK_SET);
        ret = fwrite((void*) buf, sizeof(buf), 1, f3);

        fseek(f4, BLKSZ*3, SEEK_SET);
        ret = fwrite((void*) buf, sizeof(buf), 1, f4);

        fclose(f1);
        fclose(f2);
        fclose(f3);
        fclose(f4);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

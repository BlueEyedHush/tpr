#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


#define MSG_CONF(SIZE) {SIZE, 100000000/SIZE} // total size always 10^8
#define NUM_MSG_CONFS 12
int msg_confs[][2] = {MSG_CONF(1),
                               MSG_CONF(4),
                               MSG_CONF(16),
                               MSG_CONF(64),
                               MSG_CONF(128),
                               MSG_CONF(512),
                               MSG_CONF(2048),
                               MSG_CONF(4096),
                               MSG_CONF(8192),
                               MSG_CONF(16384),
                               MSG_CONF(32768),
                               MSG_CONF(65536)};

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // We are assuming at least 2 processes for this task
    if (world_size < 2) {
        fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int number;
    if (world_rank == 0) {
        // If we are rank 0, set the number to -1 and send it to process 1
        number = -1;
        printf("Process 0 sent number %d\n", number);
        MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process 1 received number %d from process 0\n", number);
    }
    MPI_Finalize();
}
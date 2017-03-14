#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RECEIVERS_NUM 4
#define TAG 0

void broadcast(const int world_size, const int world_rank, const int *data) {
    fprintf(stderr, "BROADCASTING %d\n", *data);
    for(int i = 0; i < world_size; i++) {
        if(i != world_rank) {
            MPI_Send(data, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // We are assuming at least 2 processes for this task
    if (world_size < RECEIVERS_NUM + 1) {
        fprintf(stderr, "World size must be greater than %d\n", RECEIVERS_NUM);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (world_rank == 0) {
        // we are the broadcaster and we are sending
        srand(time(NULL));
        int data = rand();
        broadcast(world_size, world_rank, &data);
    } else if (world_rank > 0) {
        // wait for broadcast to arrive
        int buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received number %d\n", world_rank, buffer);
    }
    MPI_Finalize();
}
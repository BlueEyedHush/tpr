#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RECEIVERS_NUM 4
#define TAG 0

short use_custom_impl = 0;
short iterations = 10;

void broadcast(const int world_size, const int world_rank, const int *data) {
    fprintf(stderr, "BROADCASTING %d\n", *data);
    for(int i = 0; i < world_size; i++) {
        if(i != world_rank) {
            MPI_Send(data, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }
    }
}

void print_usage() {
    fprintf(stderr, "Usage: executable <c|b> (c - custom, i - built-in)\n");
}

int main(int argc, char** argv) {
    if(argc < 2) {
        print_usage();
        return 1;
    } else {
        switch(argv[1][0]) {
            case 'c':
                use_custom_impl = 1;
                break;
            case 'b':
                use_custom_impl = 0;
                break;
            default:
                print_usage();
                return 1;
        }
    }

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
        double start = MPI_Wtime();
        for(int i = 0; i < iterations; i++) {
            fprintf(stderr, "Sending out the broadcast\n");
            if(use_custom_impl != 0) {
                broadcast(world_size, world_rank, &data);
            } else {
                MPI_Bcast(&data, 1, MPI_INT, world_rank, MPI_COMM_WORLD);
            }
            fprintf(stderr, "Waiting for responses\n");
            for(int i = 0; i < world_size-1; i++) {
                int buffer;
                MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                fprintf(stderr, "Hearing back from %d\n", buffer);
            }
        }
        double end = MPI_Wtime();
        fprintf(stderr, "Broadcast took: %.20fs\n", (end-start)/(2*iterations)); // dividing by 2 because we cound RTT
    } else if (world_rank > 0) {
        // wait for broadcast to arrive
        int buffer = 0;
        for(int i = 0; i < 10; i++) {
            MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fprintf(stderr, "Process %d received number %d\n", world_rank, buffer);
            buffer = world_rank;
            MPI_Send(&buffer, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
            fprintf(stderr, "Process %d is sending back\n", world_rank);
        }
    }
    MPI_Finalize();
}
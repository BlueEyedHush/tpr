#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

#define MSGS_TOTAL_SIZE 100000001 // number of bytes transfered
#define MSGS_SIZE 100000000 // number of bytes of meaningful data
#define MSG_CONF(SIZE) {SIZE, 100000000/SIZE} // total size always 10^8
#define NUM_MSG_CONFS 12
const int msg_confs[][2] = {MSG_CONF(1),
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

byte buffer[MSGS_TOTAL_SIZE] = {0}; // one is reserved to store id into msg_confs array to make responding easier

MPI_Datatype datatypes[NUM_MSG_CONFS] = {0};

void register_datatypes() {
    for(int i = 0; i < NUM_MSG_CONFS; i++) {
        int size = msg_confs[i][0];
        MPI_Type_contiguous(size, MPI_BYTE, &datatypes[i]);
        MPI_Type_commit(&datatypes[i]);
    }
}

void pong_main() {
    int number = -1;
    printf("Process 0 sent number %d\n", number);
    MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
}

void ping_main() {
    int number = 0;
    MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process 1 received number %d from process 0\n", number);
}

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

    register_datatypes();

    if (world_rank == 0) {
        pong_main();
    } else if (world_rank == 1) {
        ping_main();
    } else {
        fprintf(stderr, "Proces has rank > 1 => more than 2 processes. Terminating without doing anything.\n");
        return 1;
    }

    MPI_Finalize();
    return 0;
}
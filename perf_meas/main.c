#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

#define PONG_RANK 0
#define PING_RANK 1

#define MSGS_TOTAL_SIZE 100000000 // number of bytes transfered
#define MSG_CONF(SIZE) {SIZE, 100000000/SIZE} // total size always 10^8
#define NUM_MSG_CONFS 12
#define MSG_SIZE_ID 0
#define MSG_COUNT_ID 1
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
        int size = msg_confs[i][MSG_SIZE_ID];
        MPI_Type_contiguous(size, MPI_BYTE, &datatypes[i]);
        MPI_Type_commit(&datatypes[i]);
    }
}

void pong_main() {
    fprintf(stderr, "[PONG] Entered pong_main, waiting for data to arrive\n");
    for(int i = 0; i < NUM_MSG_CONFS; i++) {
        fprintf(stderr, "[PONG] Waiting for comm session %d", i);
        MPI_Recv(buffer, msg_confs[i][MSG_COUNT_ID], datatypes[i], PING_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        fprintf(stderr, "[PONG] Received data in comm session %d", i);
        MPI_Send(buffer, msg_confs[i][MSG_COUNT_ID], datatypes[i], PING_RANK, MPI_ANY_TAG, MPI_COMM_WORLD);
        fprintf(stderr, "[PONG] Sent back data in comm session %d", i);
    }
    fprintf(stderr, "[PONG] All expected message successfully received and sent back. Terminating.\n");
}

void ping_main() {
    fprintf(stderr, "[PING] Entered pong_main, waiting for data to arrive\n");
    for(int i = 0; i < NUM_MSG_CONFS; i++) {
        fprintf(stderr, "[PING] Sending data in comm session %d", i);
        MPI_Send(buffer, msg_confs[i][MSG_COUNT_ID], datatypes[i], PONG_RANK, MPI_ANY_TAG, MPI_COMM_WORLD);
        fprintf(stderr, "[PING] Data in session %d successfully sent. Waiting for them to come back.", i);
        MPI_Recv(buffer, msg_confs[i][MSG_COUNT_ID], datatypes[i], PONG_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        fprintf(stderr, "[PING] Received data from PONG process in session %d", i);
    }
    fprintf(stderr, "[PING] All expected message successfully sent. All responses received. Terminating.\n");
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

    if (world_rank == PONG_RANK) {
        pong_main();
    } else if (world_rank == PING_RANK) {
        ping_main();
    } else {
        fprintf(stderr, "Proces has rank > 1 => more than 2 processes. Terminating without doing anything.\n");
        return 1;
    }

    MPI_Finalize();
    return 0;
}
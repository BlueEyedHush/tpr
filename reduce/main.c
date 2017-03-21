
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>

// Creates an array of random numbers. Each number has a value from 0 - 1
float *create_rand_nums(int num_elements) {
	float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
	assert(rand_nums != NULL);
	for (int i = 0; i < num_elements; i++) {
		rand_nums[i] = (rand() / (float)RAND_MAX);
	}
	return rand_nums;
}


//for now count MUST == 1
void reduce(const float *sendbuf, float *recvbuf, /*int count,*/ int root, MPI_Comm comm) {
	int count = 1;
	int tag = 0;
	int world_rank;
	MPI_Comm_rank(comm, &world_rank);

	int world_size;
	MPI_Comm_size(comm, &world_size);

	/*Sending result to root*/
	if (world_rank != root) {
		MPI_Send(sendbuf, count, MPI_FLOAT, root, tag, comm);
	}

	// Reduce all of the local sums into the global sum
	if (world_rank == root) {
		float global_sum;
		int i = 0;
		global_sum = *sendbuf;
		while (i < world_size - 1) {
			int flag = 0;
			MPI_Iprobe(MPI_ANY_SOURCE, tag, comm, &flag, MPI_STATUS_IGNORE);
			if (flag) {
				float sum = 0;
				MPI_Recv(&sum, count, MPI_FLOAT, MPI_ANY_SOURCE, tag,
					comm, MPI_STATUS_IGNORE);
				global_sum += sum;
				i++;
			}
		}
		*recvbuf = global_sum;
	}
}

void print_usage() {
	fprintf(stderr, "Usage: executable <c|i> (c - custom, i - built-in)\n");
}

int main(int argc, char** argv) {
	if (argc < 2) {
		print_usage();
		return 1;
	}
	else {
		switch (argv[1][0]) {
		case 'c':
			use_custom_impl = 1;
			break;
		case 'i':
			use_custom_impl = 0;
			break;
		default:
			print_usage();
			return 1;
		}
	}

	int num_elements_per_proc = 2;
	MPI_Init(NULL, NULL);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	// Create a random array of elements on all processes.
	srand(time(NULL)*world_rank);   // Seed the random number generator to get different results each time for each processor
	float *rand_nums = NULL;
	rand_nums = create_rand_nums(num_elements_per_proc);

	// Sum the numbers locally
	float local_sum = 0;
	for (int i = 0; i < num_elements_per_proc; i++) {
		local_sum += rand_nums[i];
	}

	// Print the random numbers on each process
	//printf("Local sum for process %d - %f\n", world_rank, local_sum);
	float global_sum;

	// Clean up
	free(rand_nums);

	MPI_Barrier(MPI_COMM_WORLD);
	double time_start = MPI_Wtime();
	for (int i = 0; i < 5; i++) {
		if (use_custom_impl != 0) {
			reduce(&local_sum, &global_sum, 0, MPI_COMM_WORLD);
		}
		else {
			MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	double time_end = MPI_Wtime();

	if (world_rank == 0) {
		printf("Reduce time: %.20fs\n", (time_end - time_start) / 10);
	}

	MPI_Finalize();
}
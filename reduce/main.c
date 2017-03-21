
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>

#define ITERS 1
#define TAG 0

short use_custom_impl = 0;

// Creates an array of random numbers. Each number has a value from 0 - 1
float *create_rand_nums(int num_elements) {
	float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
	assert(rand_nums != NULL);
	for (int i = 0; i < num_elements; i++) {
		rand_nums[i] = (rand() / (float)RAND_MAX);
	}
	return rand_nums;
}

float reduce_master(float nums[], int world_size) {
	// start computation
	for (int i = 1; i < world_size; i++) {
		MPI_Send(&nums[i - 1], 1, MPI_FLOAT, i, TAG, MPI_COMM_WORLD);
	}

	// wait for the result to be returned
	float final_result = 0.0f;
	MPI_Recv(&final_result, 1, MPI_FLOAT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	return final_result;
}

void reduce_worker(int world_rank, int world_size) {
	// assuming world_size 2^N+1, for N >= 1
	if (world_size < 3) {
		fprintf(stderr, "World size must be 2^N+1 for N >= 1, but is %d", world_size);
	}
	else {
		// initial value for reduce - 0
		float computation_result = 0.0f;
		int stride_size = world_size - 1;
		for (; world_rank < stride_size + 1; stride_size /= 2) {
			// wait for data required for computation
			//fprintf(stderr, "Process %d waiting for data, stride %d\n", world_rank, stride_size);
			float partial = 0.0f;
			MPI_Recv(&partial, 1, MPI_FLOAT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// combine partial with result we currently have
			computation_result += partial;
		}
		// we no longer need to compute - now we have to send it
		// works also for last processor -> it'll have world_rank 1, which is not < than 1 - final stride size
		int dest = world_rank == 1 ? 0 : world_rank - stride_size;
		//fprintf(stderr, "Process %d ended with stride %d, sending data to %d\n", world_rank, stride_size, dest);
		MPI_Send(&computation_result, 1, MPI_FLOAT, dest, TAG, MPI_COMM_WORLD);
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

	MPI_Init(NULL, NULL);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	double time_start;
	double time_end;

	if (use_custom_impl != 0) {
		if (world_rank == 0) {
			// Create a random array of elements on all processes.
			srand(time(NULL)*world_rank);   // Seed the random number generator to get different results each time for each processor
			int n = world_size - 1;
			float *rand_nums = create_rand_nums(n);

			// Sum the numbers locally
			float local_sum = 0;
			for (int i = 0; i < n; i++) {
				local_sum += rand_nums[i];
			}
			//fprintf(stderr, "Local sum: %.20fs\n", local_sum);

			time_start = MPI_Wtime();
			for (int i = 0; i < ITERS; i++) {
				float result = reduce_master(rand_nums, world_size);
				// fprintf(stderr, "TOTAL sum = %f\n", result);
			}
			time_end = MPI_Wtime();

			// Clean up
			free(rand_nums);
		}
		else {
			reduce_worker(world_rank, world_size);
		}
	}
	else {
		srand(time(NULL)*world_rank);   // Seed the random number generator to get different results each time for each processor
		int n = world_size - 1;
		float *rand_nums = create_rand_nums(n);

		float local_sum = 0;
		for (int i = 0; i < n; i++) {
			local_sum += rand_nums[i];
		}
		float global_sum;

		time_start = MPI_Wtime();
		for (int i = 0; i < ITERS; i++) {
			MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
		}
		time_end = MPI_Wtime();

		// Clean up
		free(rand_nums);
	}

	// Print the result
	if (world_rank == 0) {
		printf("%.20f\n", (time_end - time_start) / (world_size - 1));
	}

	MPI_Finalize();
}
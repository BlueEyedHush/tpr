#include <mpi.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

float *create_rand_nums(int num_elements) {
	float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
	assert(rand_nums != NULL);
	for (int i = 0; i < num_elements; i++) {
		rand_nums[i] = (rand() / (float)RAND_MAX);
	}
	return rand_nums;
}

void print_usage() {
	fprintf(stderr, "Usage: executable [number of iterations]\n");
}

int main(int argc, char** argv) {
	long num_iter;
	if (argc < 2) {
		print_usage();
		return 1;
	}
	else {
		num_iter = atoi(argv[1]);
	}
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the number of processes
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the rank of the process
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (world_size < 1) {
		fprintf(stderr, "World size must be greater than %d\n", 1);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	double time_start;
	double time_end;
	MPI_Barrier(MPI_COMM_WORLD);
	time_start = MPI_Wtime();

	long num_iter_per_processor = num_iter / world_size;

	double x, y;
	int local_circle_count = 0; /* # of points in the 1st quadrant of unit circle */
	double z;
	double pi;
	/* initialize random numbers */
	srand(world_rank);
	for (long i = 0; i < num_iter_per_processor; i++) {
		float *rand_nums = create_rand_nums(2);
		x = rand_nums[0];
		y = rand_nums[1];
		z = x*x + y*y;
		if (z <= 1) local_circle_count++;
	}
	int global_circle_count;

	MPI_Reduce(&local_circle_count, &global_circle_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	time_end = MPI_Wtime();

	if (world_rank == 0) {
		pi = 4.0 *(double)global_circle_count / num_iter;
		printf("# iter= %d , estimate of pi is %g \n", num_iter, pi);
		printf("%.20f\n", time_end - time_start);
	}

	// Finalize the MPI environment.
	MPI_Finalize();
}
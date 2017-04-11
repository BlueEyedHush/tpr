
#include <omp.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <climits>
#include <cmath>

#define PRINT_CONFIGURATION 1
#define PRINT_ARRAY_CONTENTS 0
#define MERGING_PARALLEL 1
#define IN_OUT_SIZE_VALIDATION 0
#define SUM_VALIDATION 0
#define EXTENDED_REPORTING 2 /* 1 enabled, 2 - with profiling info */
#define FINE_GRAINED_LOCKING 1

#define VERSION "02ee50aad0"
#define MAX_VALUE 1000.0
#define EPSILON 1e-6

using namespace std;

struct predicate {
	bool operator()(double i, double j) { return (i < j); }
} pred;

//Prints array
// Parameters:
// data - array of integers to search from
// count - elements in data array
static void print_array(double *data, int count) {
	printf("[");
	for (int i = 0; i < count; i++)
		if (i != count - 1) {
			printf("%f, ", data[i]);
		} else {
			printf("%f", data[i]);
		}

	printf("]");
}

typedef struct times {
	float mid;
	float end;
} times;

static times bucket_sort(double *data, int dataN, int bucketCount) {
	#if SUM_VALIDATION == 1
		long double sum_before_sorting = 0.0;
		for(int i = 0; i < dataN; i++) {
			sum_before_sorting += data[i];
		}
	#endif

	/* array of pointer to buckets */
	vector<double> **buckets = new vector<double> *[bucketCount];
	#if defined(_OPENMP)
	omp_lock_t *locks = new omp_lock_t[bucketCount];
	#endif

	// Creates all buckets
	#pragma omp parallel for
	for (int i = 0; i < bucketCount; i++) {
		buckets[i] = new vector<double>();

		#if defined(_OPENMP) && FINE_GRAINED_LOCKING == 1
		omp_init_lock(&(locks[i]));
		#endif
	}

	const clock_t begin_time = clock();

	// Populates buckets with data
	/*
	 * How to calculate which bucket number should go to?
	 * id = value/bucketWidth
	 *
	 * How to calculate bucket width?
	 * bucketWidth = maxValue/bucketCount
	 *
	 * Problem:
	 * when value == maxValue,
	 * id == value/(maxValue/bucketCount) == value*bucketCount/maxValue == bucketCount
	 * but max allowed id is bucketCount-1
	 * Solution: increase maxValue by 1
	 * */

	#pragma omp parallel for
	for (int i = 0; i < dataN; i++) {
		int selectedBucket = int ((data[i] * bucketCount) / (MAX_VALUE+1));
		#if defined(_OPENMP)
			#if FINE_GRAINED_LOCKING == 1
				omp_set_lock(&(locks[selectedBucket]));
				buckets[selectedBucket]->push_back(data[i]);
				omp_unset_lock(&(locks[selectedBucket]));
			#else
				#pragma omp critical
				buckets[selectedBucket]->push_back(data[i]);
			#endif
		#else
			buckets[selectedBucket]->push_back(data[i]);
		#endif
	}

	clock_t middle_time = clock();

	#if MERGING_PARALLEL == 1
		#if IN_OUT_SIZE_VALIDATION == 1
			int insertedElements = 0;
		#endif

		#pragma omp parallel for
		for (int i = 0; i < bucketCount; i++)
		{
			// sort bucket
			sort(buckets[i]->begin(), buckets[i]->end(), pred);

			// Merges buckets to array
			size_t currBucketSize = buckets[i]->size();
			if(currBucketSize > 0) {
				// calculate offset
				int nextElementId = 0;
				for(int j = 0; j < i; j++) {
					nextElementId += buckets[j]->size();
				}

				// Copy all values from bucket to array
				for (size_t j = 0; j < currBucketSize; j++)
				{
					data[nextElementId] = buckets[i]->at(j);
					nextElementId++;
				}

				#if IN_OUT_SIZE_VALIDATION == 1
					#pragma omp atomic update
					insertedElements += currBucketSize;
				#endif
			}
		}
	#else
		#pragma omp parallel for
		// Sort all buckets
		for (int i = 0; i < bucketCount; i++) {
			sort(buckets[i]->begin(), buckets[i]->end(), pred);
		}

		// Merges buckets to array
		int insertedElements = 0;
		for (int i = 0; i < bucketCount; i++) {
			// Copy all values from bucket to array
			for (size_t j = 0; j < buckets[i]->size(); j++) {
				data[insertedElements] = buckets[i]->at(j);
				insertedElements++;
			}
		}
	#endif
	const float end_time = clock();

	times result;
	result.mid = (float(middle_time - begin_time)) / CLOCKS_PER_SEC;
	result.end = (float(end_time - begin_time)) / CLOCKS_PER_SEC;

	for (int i = 0; i < bucketCount; i++) {
		delete buckets[i];
		#if defined(_OPENMP) && FINE_GRAINED_LOCKING == 1
			omp_destroy_lock(&(locks[i]));
		#endif
	}
	#if defined(_OPENMP) && FINE_GRAINED_LOCKING == 1
		delete[] locks;
	#endif
	delete[] buckets;

	#if IN_OUT_SIZE_VALIDATION == 1
		if (insertedElements != dataN) {
			printf("[ERROR] output array has less elements than input array (out: %d, in: %d) !!!\n", insertedElements, dataN);
		}
	#endif

	#if SUM_VALIDATION == 1
		long double sum_after_sorting = 0.0;
		for(int i = 0; i < dataN; i++) {
			sum_after_sorting += data[i];
		}

		long double diff = fabs(sum_before_sorting - sum_after_sorting);
		if(diff > EPSILON) {
			printf("[ERROR] sums before and after differ by (%Lf.10) !!!\n", diff);
		}
	#endif

	return result;
}


double *generate_random_array(int size, double from, double to, int seed) {

	std::seed_seq seed_sequence = {seed};
	std::mt19937 rng(seed_sequence);
	std::uniform_real_distribution<double> uni(from, to);

	double *arr = new double[size];
	for (int i = 0; i < size; i++) {
		arr[i] = uni(rng);
	}
	return arr;
}

int main(int argc, char* argv[]) {
	if(argc < 5) {
		printf("Usage: executable <array_size> <bucket_count> <seed> <iterations>\n");
		return 1;
	}

	int array_size = atoi(argv[1]);
	int bucket_count = atoi(argv[2]);
	int rand_seed = atoi(argv[3]);
	int iterations = atoi(argv[4]);

	#if defined(_OPENMP)
		int thread_num = -2;
		#pragma omp parallel
		thread_num =  omp_get_num_threads();
	#else
		int thread_num = -1;
	#endif

	#if PRINT_CONFIGURATION == 1
		printf("Version: %s\n", VERSION);
		printf("Array size: %15d, bucket count: %5d, seed: %d, iterations: %d, thread_num: %3d\n",
		       array_size, bucket_count, rand_seed, iterations, thread_num);
	#endif

	double *unsorted = generate_random_array(array_size, 1, MAX_VALUE, rand_seed);

	#if PRINT_ARRAY_CONTENTS == 1
		print_array(unsorted, array_size);
		printf("\n");
	#endif

	// first iteration, non-timed (warmup)
	bucket_sort(unsorted, array_size, bucket_count);

	// timed iterations
	for(int i = 0; i < iterations; i++) {
		const times t = bucket_sort(unsorted, array_size, bucket_count);

		#if EXTENDED_REPORTING == 2
			printf("%d\t%d\t%d\t%.20f\t%.1f\t%.20f\n", array_size, bucket_count, thread_num, t.mid, t.mid/t.end, t.end);
		#elif EXTENDED_REPORTING == 1
			printf("%d\t%d\t%d\t%.20f\n", array_size, bucket_count, thread_num, duration);
		#else
			printf("%.20f\n", duration);
		#endif
	}

	#if PRINT_ARRAY_CONTENTS == 1
		print_array(unsorted, array_size);
	#endif

	delete[] unsorted;
	return 0;
}



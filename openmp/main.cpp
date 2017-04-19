
#include <omp.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <climits>
#include <cmath>

#define PRINT_CONFIGURATION 0
#define PRINT_ARRAY_CONTENTS 0
#define MERGING_PARALLEL 1
#define IN_OUT_SIZE_VALIDATION 0
#define SUM_VALIDATION 0
#define EXTENDED_REPORTING 2 /* 1 enabled, 2 - with profiling info */
#define FINE_GRAINED_LOCKING 1
#define CALC_AVERAGE 0
#define PRINT_HEADER 0
#define SORTED_VALIDATION 1

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

typedef struct timestamps {
	clock_t start;
	clock_t mid;
	clock_t end;
} timestamps;

timestamps *init_times(timestamps *t) {
	t->start = 0;
	t->mid = 0;
	t->end = 0;
	return t;
}

float get_elasped_time(clock_t start, clock_t end) {
	return (float(end - start)) / CLOCKS_PER_SEC;
}

static void bucket_sort(double *data, int dataN, int bucketCount, timestamps *out_ts) {
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

	out_ts->start = clock();

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

	out_ts->mid = clock();

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
	out_ts->end = clock();

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
}


void fill_array(double *array, int size, double from, double to, int seed) {

	std::seed_seq seed_sequence = {seed};
	std::mt19937 rng(seed_sequence);
	std::uniform_real_distribution<double> uni(from, to);

	for (int i = 0; i < size; i++) {
		array[i] = uni(rng);
	}
}

short verify_sorted(double *array, int size) {
	short not_sorted = 0;
	for (int i = 0; i < size-1 && not_sorted == 0; i++) {
		if (array[i] > array[i+1]) {
			printf("[ERROR] not sorted, encountered problem at position %d", i);
			not_sorted = 1;
		}
	}
	return (short) (1 - not_sorted);
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

	double *unsorted = new double[array_size];
	timestamps tmp_ts;

	int tsN = (CALC_AVERAGE == 1) ? 1 : iterations;
	float *bucket_filling_times = new float[tsN];
	float *sorting_and_merging_times = new float[tsN];

	for (int i = 0; i < tsN; i++) {
		bucket_filling_times[i] = 0.0f;
		sorting_and_merging_times[i] = 0.0f;
	}

	for(int i = 0; i < iterations; i++) {
		init_times(&tmp_ts);
		fill_array(unsorted, array_size, 1, MAX_VALUE, rand_seed);

		#if PRINT_ARRAY_CONTENTS == 1
			print_array(unsorted, array_size);
			printf("\n");
		#endif

		bucket_sort(unsorted, array_size, bucket_count, &tmp_ts);

		#if SORTED_VALIDATION == 1
			verify_sorted(unsorted, array_size);
		#endif

		float bf_time = get_elasped_time(tmp_ts.start, tmp_ts.mid);
		float sam_time = get_elasped_time(tmp_ts.mid, tmp_ts.end);

		int target_index = (CALC_AVERAGE == 1) ? 0 : i;
		bucket_filling_times[target_index] += bf_time;
		sorting_and_merging_times[target_index] += sam_time;
	}

	#if CALC_AVERAGE == 1
		bucket_filling_times[0] /= iterations;
		sorting_and_merging_times[0] /= iterations;
	#endif

	#if PRINT_HEADER == 1
		printf("%15.15s %15.15s %9.9s %25.25s %25.25s %12.12s %25.25s\n", "ARRAY_SIZE", "#BUCKETS", "#THREADS", "T_1", "T_2",
		       "T_1/T_TOTAL", "T_TOTAL");
	#endif
	for(int i = 0; i < tsN; i++) {
		float bf_time = bucket_filling_times[i];
		float sam_time = sorting_and_merging_times[i];
		float total = bf_time + sam_time;
		#if EXTENDED_REPORTING == 2
			printf("%15d %15d %9d %25.20f %25.20f %12.2f %25.20f\n", array_size, bucket_count, thread_num, bf_time, sam_time,
			       bf_time/total, total);
		#elif EXTENDED_REPORTING == 1
			printf("%d\t%d\t%d\t%.20f\n", array_size, bucket_count, thread_num, total);
		#else
			printf("%.20f\n", t.end);
		#endif
	}

	#if PRINT_ARRAY_CONTENTS == 1
		print_array(unsorted, array_size);
	#endif

	delete[] unsorted;
	return 0;
}



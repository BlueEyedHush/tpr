
#include <omp.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <climits>

#define PRINT_CONFIGURATION 0
#define PRINT_ARRAY_CONTENTS 0
#define MERGING_PARALLEL 0
#define IN_OUT_SIZE_VALIDATION 0

using namespace std;

struct predicate {
	bool operator()(int i, int j) { return (i < j); }
} pred;

//Prints array
// Parameters:
// data - array of integers to search from
// count - elements in data array
static void print_array(int *data, int count) {
	printf("[");
	for (int i = 0; i < count; i++)
		if (i != count - 1) {
			printf("%d, ", data[i]);
		} else {
			printf("%d", data[i]);
		}

	printf("]");
}

// Finds maximum value in data array
static int find_max(int *data, int count) {
	int max = INT_MIN;

	for (int i = 0; i < count; i++) {
		if (data[i] > max)
			max = data[i];
	}

	return max;
}

static void bucket_sort(int *data, int dataN, int bucketCount) {
	/* array of pointer to buckets */
	vector<int> **buckets = new vector<int> *[bucketCount];

	// Creates all buckets
	#pragma omp parallel for
	for (int i = 0; i < bucketCount; i++) {
		buckets[i] = new vector<int>();
	}

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

	const int maxValue = find_max(data, dataN) + 1;
	#pragma omp parallel for
	for (int i = 0; i < dataN; i++) {
		int selectedBucket = (data[i] * bucketCount) / maxValue;
		#pragma omp critical
		buckets[selectedBucket]->push_back(data[i]);
	}

	#pragma omp parallel for
	// Sort all buckets
	for (int i = 0; i < bucketCount; i++) {
		sort(buckets[i]->begin(), buckets[i]->end(), pred);
	}

	// Merges buckets to array
	#if MERGING_PARALLEL == 1
		#if IN_OUT_SIZE_VALIDATION == 1
			int insertedElements = 0;
		#endif

		#pragma omp parallel for
		for (int i = 0; i < bucketCount; i++)
		{
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
		int insertedElements = 0;
		for (int i = 0; i < bucketCount; i++) {
			// Copy all values from bucket to array
			for (size_t j = 0; j < buckets[i]->size(); j++) {
				data[insertedElements] = buckets[i]->at(j);
				insertedElements++;
			}
		}
	#endif

	#if IN_OUT_SIZE_VALIDATION == 1
		if (insertedElements != dataN) {
			printf("[ERROR] output array has less elements than input array (out: %d, in: %d) !!!\n", insertedElements, dataN);
		}
	#endif
}


int *generate_random_array(int size, int from, int to, int seed) {

	std::seed_seq seed_sequence = {seed};
	std::mt19937 rng(seed_sequence);
	std::uniform_int_distribution<int> uni(from, to);

	int *arr = new int[size];
	for (int i = 0; i < size; i++) {
		arr[i] = uni(rng);
	}
	return arr;
}

int main(int argc, char* argv[]) {
	if(argc < 4) {
		printf("Usage: executable <array_size> <bucket_count> <seed>\n");
		return 1;
	}

	int array_size = atoi(argv[1]);
	int bucket_count = atoi(argv[2]);
	int rand_seed = atoi(argv[3]);

	#if PRINT_CONFIGURATION == 1
		printf("Array size: %d, bucket count: %d, seed: %d, print contents: %d, parallel merging: %d, size validation: %d\n",
		       array_size, bucket_count, rand_seed, PRINT_ARRAY_CONTENTS, MERGING_PARALLEL, IN_OUT_SIZE_VALIDATION);
	#endif

	int *unsorted = generate_random_array(array_size, 1, 1000, rand_seed);

	#if PRINT_ARRAY_CONTENTS == 1
		print_array(unsorted, array_size);
		printf("\n");
	#endif

	const clock_t begin_time = clock();
	bucket_sort(unsorted, array_size, bucket_count);
	const clock_t end_time = clock();
	printf("%.20f\n", (float(end_time - begin_time)) / CLOCKS_PER_SEC);

	#if PRINT_ARRAY_CONTENTS == 1
		print_array(unsorted, array_size);
	#endif

	getchar();
	return 0;
}



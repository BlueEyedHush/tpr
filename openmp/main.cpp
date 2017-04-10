
#include <omp.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <climits>

#define PRINT_ARRAY_CONTENTS 0
#define MERGING_PARALLEL 0

using namespace std;

struct predicate {
	bool operator() (int i, int j) { return (i<j); }
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
		}
		else {
			printf("%d", data[i]);
		}

		printf("]");
}

// Finds maximum value in data array
static int find_max(int *data, int count) {
	int max = INT_MIN;

	for (int i = 0; i < count; i++)
	{
		if (data[i] > max)
			max = data[i];
	}

	return max;
}

static void bucket_sort(int* data, int count) {
	int maxValue = find_max(data, count);

	int bucketCount = count;
	/* array of pointer to buckets */
	vector<int>** buckets = new vector<int>*[bucketCount];

	// Creates all buckets
	#pragma omp parallel for
	for (int i = 0; i < bucketCount; i++)
	{
		buckets[i] = new vector<int>();
	}

	// Populates buckets with data
	#pragma omp parallel for
	for (int i = 0; i < count; i++)
	{
		int selectedBucket = (data[i] * count) / (maxValue + 1);
		#pragma omp critical
		buckets[selectedBucket]->push_back(data[i]);
	}

	#pragma omp parallel for
	// Sort all buckets
	for (int i = 0; i < bucketCount; i++)
	{
		sort(buckets[i]->begin(), buckets[i]->end(), pred);
	}

	// Merges buckets to array
    #if MERGING_PARALLEL == 1
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
            }
        }
    #else
		int insertedElements = 0;
		for (int i = 0; i < bucketCount; i++)
		{
			// Copy all values from bucket to array
			for (size_t j = 0; j < buckets[i]->size(); j++)
			{
				data[insertedElements] = buckets[i]->at(j);
				insertedElements++;
			}
		}
    #endif
}


int* generate_random_array(int size, int from, int to) {

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(from, to);

	int *arr = new int[size];
	for (int i = 0; i < size; i++) {
		arr[i] = uni(rng);
	}
	return arr;
}


static const int ARRAY_SIZE = 1000000;
int main()
{
	int *unsorted = generate_random_array(ARRAY_SIZE, 1, 1000);

    #if PRINT_ARRAY_CONTENTS == 1
        print_array(unsorted, ARRAY_SIZE);
        printf("\n");
    #endif

	const clock_t begin_time = clock();
	bucket_sort(unsorted, ARRAY_SIZE);
	const clock_t end_time = clock();
	printf("Sorted %d elements in %f seconds\n", ARRAY_SIZE, (float (end_time - begin_time))/CLOCKS_PER_SEC);

    #if PRINT_ARRAY_CONTENTS == 1
	    print_array(unsorted, ARRAY_SIZE);
    #endif

	getchar();
	return 0;
}



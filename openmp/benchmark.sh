#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

# miscellaneous constants
RESULT_FILE="$DIR"/results.out

BUCKETS="50 5000 25000 125000 1000000"
THREADS="1 2 4 5 6 7 8 10 12 14"
SIZES="100000 1000000 10000000"
ITERATIONS=3
SEEDS=5

# load required modules
module load plgrid/tools/gcc/

# compile in 2 versions: with and without OpenMP
echo "compiling"

WITHOUT_EXEC="$DIR"/main
WITH_EXEC="$DIR"/main_omp

g++ -o "$WITH_EXEC" -fopenmp -std=c++11 -O3 "$DIR"/main.cpp
g++ -o "$WITHOUT_EXEC" -std=c++11 -O3 "$DIR"/main.cpp

# gather data
echo "running"

for bucket in $BUCKETS; do
    for size in $SIZES; do
        for seed in $SEEDS; do
            "$WITHOUT_EXEC" $size $bucket $seed $ITERATIONS | tee -a "$RESULT_FILE"

            for thread in $THREADS; do
                export OMP_NUM_THREADS=$thread
                "$WITH_EXEC" $size $bucket $seed $ITERATIONS | tee -a "$RESULT_FILE"
            done
        done
    done
done
#!/usr/bin/env bash

# miscellaneous constants
RESULT_FILE=results.out

BUCKETS="1000000"
THREADS="4 8 12"
SIZES="100000000"
ITERATIONS=1
SEEDS=7892773

if [ -z $1 ]; then
    echo "Username (without plg prefix) must be passed as first argument"
    exit 1
else
    USER=$1
fi

# load required modules
module load plgrid/tools/gcc/5.4.0

if [ ! -z $TPR_DEBUG ]; then
    DIR="."
else
    DIR="/people/plg${USER}/openmp"
fi

echo "$DIR"
pushd "$DIR"
# compile in 2 versions: with and without OpenMP
echo "compiling"

WITHOUT_EXEC="./main"
WITH_EXEC="./main_omp"

g++ -o "$WITH_EXEC" -fopenmp -std=c++11 -O3 main.cpp
g++ -o "$WITHOUT_EXEC" -std=c++11 -O3 main.cpp

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

popd
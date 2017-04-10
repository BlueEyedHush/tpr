#!/bin/bash

#PBS -l walltime=0:10:00
#PBS -l pmem=64mb
#PBS -l nodes=2:ppn=12
#PBS -N borysenkosort
#PBS -A plgborysenko2017a

# miscellaneous constants
RESULT_FILE="results.out"

BUCKETS="12 24 48"
THREADS="1 2 4 6 12"
SIZES="1000000 10000000 100000000"
ITERATIONS=1
SEEDS=5

# load required modules
module load plgrid/tools/gcc/

pushd /people/plgborysenko/openmp

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
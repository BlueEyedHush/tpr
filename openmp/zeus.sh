#PBS -l nodes=2:ppn=12
#!/bin/sh
#PBS -l walltime=0:10:00
#PBS -l pmem=64mb
#PBS -l nodes=2:ppn=12
#PBS -N borysenkosort
#PBS -A plgborysenko2017a
module load plgrid/tools/gcc/
cd /people/plgborysenko/openmp
g++ -o main -fopenmp -std=c++11 -O3 main.cpp
treads=4
size=100000000
buckets=100
seed=5
export OMP_NUM_THREADS=$treads
./main $size $buckets $seed | tee sort.$size.elements.in.$treads.threads.with.$buckets.buckets.log
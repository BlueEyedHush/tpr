#!/bin/sh
#PBS -l walltime=0:10:00
#PBS -l pmem=64mb
#PBS -l nodes=1:ppn=4
#PBS -N borysenkopi
#PBS -A plgborysenko2017a
module load plgrid/tools/mvapich2
module load plgrid/tools/mpich
module load plgrid/tools/mpiexec
cd /people/plgborysenko/pi
mpicc -std=c99 -o pi main.c
proc=4
iter=1000000000
mpiexec -np $proc ./pi $iter | tee "pi_run.proc.$proc.iter.$iter.log"

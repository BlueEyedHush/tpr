#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Need to specify project as first argument"
    exit 1
else
    PROJ="$1"
fi

DIR="$(dirname "${BASH_SOURCE[0]}")"
PROJ_DIR="$DIR/$PROJ/"
OUT_DIR="$DIR/out/$PROJ/"

mkdir -p "$OUT_DIR"

if [ "$PROJ" == "perf_meas" ]; then
    mpicc "$PROJ_DIR"main.c -cc=gcc -o "$OUT_DIR"sync -D SYNC -std=c99
    mpicc "$PROJ_DIR"main.c -cc=gcc -o "$OUT_DIR"buff -D BUFFERED -std=c99
    mpicc "$PROJ_DIR"main.c -cc=gcc -o "$OUT_DIR"sync_lat -D SYNC -D LATENCY -std=c99
    mpicc "$PROJ_DIR"main.c -cc=gcc -o "$OUT_DIR"buff_lat -D BUFFERED -D LATENCY -std=c99
elif [ "$PROJ" == "openmp" ]; then
    g++ -o "$OUT_DIR"exec -fopenmp -std=c++11 -Wall -Wextra "$PROJ_DIR"main.cpp
else
    mpicc "$PROJ_DIR"main.c -cc=gcc -o "$OUT_DIR"exec -std=c99
fi
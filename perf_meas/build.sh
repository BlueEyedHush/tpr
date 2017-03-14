#!/usr/bin/env bash

mkdir -p out/
mpicc main.c -cc=gcc -o out/perfm_sync -D SYNC
mpicc main.c -cc=gcc -o out/perfm_buff -D BUFFERED

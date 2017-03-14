#!/usr/bin/env bash

DIR="$(dirname "${BASH_SOURCE[0]}")"

mkdir -p "$DIR"/out/
mpicc main.c -cc=gcc -o "$DIR"/out/perfm_sync -D SYNC -std=c99
mpicc main.c -cc=gcc -o "$DIR"/out/perfm_buff -D BUFFERED -std=c99

mpicc main.c -cc=gcc -o "$DIR"/out/perfm_sync_lat -D SYNC -D LATENCY -std=c99
mpicc main.c -cc=gcc -o "$DIR"/out/perfm_buff_lat -D BUFFERED -D LATENCY -std=c99

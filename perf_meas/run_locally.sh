#!/usr/bin/env bash

DIR="$(dirname "${BASH_SOURCE[0]}")"

mpiexec -np 2 "$DIR"/out/perfm_sync
mpiexec -np 2 "$DIR"/out/perfm_buff

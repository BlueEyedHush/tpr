#!/usr/bin/env bash

DIR="$(dirname "${BASH_SOURCE[0]}")"

mpiexec -machinefile "$PBS_NODEFILE" -np 2 "$DIR"/out/perfm_sync
mpiexec -machinefile "$PBS_NODEFILE" -np 2 "$DIR"/out/perfm_buff
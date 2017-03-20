#!/usr/bin/env bash

DIR="$(dirname "${BASH_SOURCE[0]}")"

mpiexec -np 5 "$DIR"/out/broadcast_custom
mpiexec -np 5 "$DIR"/out/broadcast

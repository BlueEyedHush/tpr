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

mpiexec -np 5 "$OUT_DIR"exec
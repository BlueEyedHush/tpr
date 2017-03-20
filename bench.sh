#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Need to specify project as first argument"
    exit 1
else
    PROJ="$1"
fi

if [ -z "$2" ]; then
    echo "Need to specify environment local|vcluster as second argument"
    exit 1
else
    ENV="$2"
fi

if [ -z "$3" ]; then
    echo "Need to number of iterations as third argument"
    exit 1
else
    ITERS="$3"
fi

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"
PROJ_DIR="$DIR/$PROJ/"
OUT_DIR="$DIR/out/$PROJ/"

RES_DIR="${DIR}/results/${PROJ}/"
mkdir -p "$RES_DIR"

RES_FILE="$RES_DIR"/res

for i in `seq 1 $ITERS`; do
    echo -e "\nNew run\n" >> "$RES_FILE"
    "$DIR"/run.sh "$PROJ" "$ENV" | tee -a "$RES_FILE"
done

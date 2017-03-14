#!/usr/bin/env bash

DIR="$(dirname "${BASH_SOURCE[0]}")"
RES_DIR="$DIR"/results
mkdir -p "$RES_DIR"

N1="$RES_DIR"/n1
N2="$RES_DIR"/n2

for i in "$N1" "$N2"; do
    echo -e "\nNew run\n" >> "$i"
done

"$DIR"/run_locally.sh >> "$N1"
"$DIR"/run_vcluster.sh >> "$N2"
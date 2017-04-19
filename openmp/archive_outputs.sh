#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

mkdir -p "$DIR"/archive_results
mv "$DIR"/*sort.* "$DIR"/archive_results/
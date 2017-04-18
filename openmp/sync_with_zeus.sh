#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

rsync -avzr "$DIR"/. plgblueeyedhush@zeus.cyfronet.pl:/people/plgblueeyedhush/tpr/omp

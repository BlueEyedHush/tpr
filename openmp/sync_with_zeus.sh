#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

sshpass -p 97e03130B234 rsync -avzr "$DIR"/. plgblueeyedhush@zeus.cyfronet.pl:/people/plgblueeyedhush/openmp

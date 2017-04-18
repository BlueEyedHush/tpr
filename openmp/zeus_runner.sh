#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

USER="borysenko"
if [ ! -z $TPR_USER ]; then
    USER=$TPR_USER
fi

CMD="qsub -q plgrid-testing -l walltime=0:10:00 -l mem=1gb -l nodes=1:ppn=12 -N ${USER}sort -A plg${USER}2017a -F $USER $DIR/benchmark.sh"
echo "$CMD"
$CMD

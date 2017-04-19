#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

"$DIR"/archive_outputs.sh

USER="borysenko"
if [ ! -z $TPR_USER ]; then
    USER=$TPR_USER
fi

CMD="qsub -q plgrid-testing -l walltime=0:10:00 -l mem=2gb -l nodes=1:ppn=12 -N ${USER}sort -A plg${USER}2017a -F $USER $DIR/benchmark.sh"
echo "$CMD"
$CMD

nohup "$DIR"/wait_and_email.sh $USER "$DIR" &
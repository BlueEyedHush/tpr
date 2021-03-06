#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Need to specify project as first argument"
    exit 1
else
    PROJ="$1"
fi

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"
PROJ_DIR="$DIR/$PROJ/"
OUT_DIR="$DIR/out/$PROJ/"

if [ "$2" == "local" ]; then
    VCLUSTER_OPTS=""
elif [ "$2" == "vcluster" ]; then
    VCLUSTER_OPTS="-machinefile vcluster_hosts"
else
    echo "Need to specify target (local|vcluster) as second argument"
    exit 1
fi

if [ -z "$3" ]; then
    echo "Third argument must be number of processes"
    exit 1
else
    PROC_NUM="$3"
fi

shift
shift
shift

CMD="mpiexec $VCLUSTER_OPTS -n $PROC_NUM ${OUT_DIR}exec $ARGS $@"
# echo "$CMD"

$CMD
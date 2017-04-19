#!/usr/bin/env bash

# usage: script <user> <dir>
USER=$1
DIR="$2"

RESULT="non_empty"
while [ ! -z "$RESULT" ]; do
    RESULT=`qstat | grep $USER`
    sleep 10
done
{ date; cat "$DIR"/${USER}sort.o*; } | mail -s "results" "knawara112@gmail.com"
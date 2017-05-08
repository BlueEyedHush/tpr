#!/usr/bin/env bash

REL_DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(readlink -e $REL_DIR)"

g++ -E "$DIR"/main.cpp -std=c++11 -fopenmp | grep -v -e "^#\ [[:digit:]]" >  preprocessed.cpp

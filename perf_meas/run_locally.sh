#!/usr/bin/env bash

mpiexec -np 2 out/perfm_sync
mpiexec -np 2 out/perfm_buff

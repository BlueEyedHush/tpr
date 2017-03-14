#!/usr/bin/env bash

mpiexec -np 2 --hostfile vcluster_hosts out/perfm_sync
mpiexec -np 2 --hostfile vcluster_hosts out/perfm_buff
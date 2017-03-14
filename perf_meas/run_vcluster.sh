#!/usr/bin/env bash

DIR="$(dirname "${BASH_SOURCE[0]}")"

#mpiexec -machinefile vcluster_hosts -np 2 "$DIR"/out/perfm_sync
#mpiexec -machinefile vcluster_hosts -np 2 "$DIR"/out/perfm_buff
mpiexec -machinefile vcluster_hosts -np 2 "$DIR"/out/perfm_sync_lat
mpiexec -machinefile vcluster_hosts -np 2 "$DIR"/out/perfm_buff_lat
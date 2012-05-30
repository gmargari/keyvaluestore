#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh

statsfolder=$1
        
./collect-stats.sh $statsfolder &&
./create-eps-per-method-plot.sh $statsfolder &&
./create-eps-all-methods-plot.sh $statsfolder &&
./create-eps-memsize-per-method.sh $statsfolder &&
./create-eps-memsize-all-methods-per-memsize.sh $statsfolder &&
./create-eps-memsize-all-methods-all-memsizes.sh $statsfolder &&
./create-eps-thrput-latency-per-method.sh $statsfolder &&
./create-eps-thrput-latency-all-methods-per-putthrput.sh $statsfolder &&
./create-eps-thrput-latency-all-methods-all-putthrputs.sh $statsfolder &&
./create-eps-thrput-latency-all-methods-all-getsizes.sh $statsfolder &&
./create-eps-thrput-latency-all-methods-all-getthrputs.sh $statsfolder &&
#./create-eps-thrput-latency-all-methods-all-numthreads.sh $statsfolder &&
./create-eps-keys-zipf-all-in-one.sh $statsfolder &&
./create-eps-keys-ordered-all-in-one.sh $statsfolder &&
./create-eps-diff-mem-datasize-same-scale.sh $statsfolder &&
./create-eps-get-latency-num-runs.sh $statsfolder &&
./create-eps-rangemerge-blocksize.sh $statsfolder &&
#./create-eps-rangemerge-flushmem.sh $statsfolder &&
./create-eps-cassandra.sh $statsfolder &&
./create-eps-thrput-latency-rangemerge-blocksizes.sh $statsfolder &&
./create-eps-memsize-all-methods-all-memsizes.sh $statsfolder &&
exit 0

echo "Error running $0!"
exit 1

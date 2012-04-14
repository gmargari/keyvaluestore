#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1
        
./collect-stats.sh $statsfolder &&
./create-eps-all-methods-plot.sh $statsfolder &&
./create-eps-all-methods-bars-per-memsize.sh $statsfolder &&
./create-eps-per-method-plot.sh $statsfolder &&
./create-eps-per-method-bars-per-memsize.sh $statsfolder &&
./create-eps-thrput-latency-three-in-one.sh $statsfolder &&
./create-eps-thrput-latency-per-method.sh $statsfolder &&
./create-eps-diff-mem-datasize-same-scale.sh $statsfolder &&
./create-eps-get-latency-num-runs.sh $statsfolder &&
./create-eps-blocksize-flushmem-bars.sh $statsfolder &&
./create-eps-ordered-keys-bars-per-method.sh $statsfolder &&
./create-eps-zipf-keys-all-in-one.sh $statsfolder &&
./create-eps-ordered-keys-all-in-one.sh $statsfolder &&
exit 0

echo "Error running $0!"

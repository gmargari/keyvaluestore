#!/bin/bash

statsfolder="/tmp/kvstore-stats"
#./run-experiments.sh $statsfolder 
./collect-stats.sh $statsfolder 
./create-eps-all-methods-comparison.sh $statsfolder 
./create-eps-per-method.sh $statsfolder 
./create-pdf.sh $statsfolder 
exit 0

echo "Error running $0!"

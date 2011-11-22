#!/bin/bash

mkdir -p /tmp/kvstore
rm -rf /tmp/kvstore/* ;
N=10
for i in `seq 1 $N`; do
    echo "======= kvstoretest - execution $i of $N ======"
    ./build/src/test/kvstoretest 2>&1 > /tmp/A
    if [ -z "`grep 'Everything ok!' /tmp/A`" ]; then
        echo "*** ERROR ***"
        cat /tmp/A
        exit 1
    fi
    rm -rf /tmp/kvstore/* ;
done

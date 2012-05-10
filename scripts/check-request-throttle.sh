#!/bin/sh

for i in `seq 1 20`; do
    echo "========== $i =========="
    /tmp/build/src/test/requestthrottletest && continue
    echo "ERROR"
    break
done

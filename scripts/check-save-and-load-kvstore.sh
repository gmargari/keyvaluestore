#!/bin/bash

memsize=2
getthreads=2
insbytes=10
keysize=100
inputfile="/tmp/delete.kvtoinsert"

function execute_sim() {
    startline=$1
    endline=$2

    numlines=$(( $endline - $startline + 1 ))

    echo "head -n $endline $inputfile | tail -n $numlines | ./build/src/sim -c $cmanager -m $memsize -s -g $getthreads"
          head -n $endline $inputfile | tail -n $numlines | ./build/src/sim -c $cmanager -m $memsize -s -g $getthreads 2> /tmp/err > /dev/null && return 0
    
    echo "Error!"
    exit 1
}

function index_md5sum() {
    ./build/src/tools/merge `cat /tmp/kvstore/dstore.info | grep "^\/tmp"` | grep -v ^"#" | awk '{print $1, $2}' | md5sum | awk '{print $1}'
}


mkdir -p /tmp/kvstore/
rm -f /tmp/kvstore/* &&

./build/src/sim -c nomerge -i $insbytes -e -k $keysize 2> /tmp/err | grep -v "^#" > $inputfile
FLINES=`cat $inputfile | wc -l` &&

for cmanager in nomerge logarithmic geometric "rangemerge -b 2" immediate cassandra; do

    echo -e "====== $cmanager ======\n"

    rm -f /tmp/kvstore/* &&
    execute_sim 1 5
    execute_sim 6 500
    execute_sim 501 1000
    execute_sim 1001 3000
    execute_sim 3001 $FLINES
    MD5A=$( index_md5sum )
    echo $MD5A

    rm -f /tmp/kvstore/* &&
    execute_sim 1 $FLINES
    MD5B=$( index_md5sum )
    echo $MD5B

    if [ "$MD5A" == "$MD5B" ]; then
        echo -e "ok\n"
    else
        echo -e "\nERROR! md5sums are different!\n"
        exit 1
    fi
done

rm $inputfile

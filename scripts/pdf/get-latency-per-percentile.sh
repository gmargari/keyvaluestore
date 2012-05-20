#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh

statsfolder=$1

pthrput=0

methods=(
"nomerge"
"cassandra-l-4"
"cassandra-l-3"
"cassandra-l-2"
"geometric-r-2"
"geometric-r-3"
"geometric-r-4"
"geometric-p-4"
"geometric-p-3"
"geometric-p-2"
"rangemerge"
"immediate"
)

# percentile = 0 -> print average time
percentiles=( "0" "0.90" "0.99" "0.999" "1" )

# print header
echo "" | awk '{printf "\n%-20s ", $0}'
for p in ${percentiles[@]}; do
    if [ $p == "0" ]; then
        echo "avg" | awk '{printf "%6s ", $1}'
    else
        echo $p | awk '{printf "%6.1f ", $1*100}'
    fi
done 
echo -e "\n"

# print: method - get latencies
tmpfile="$(mktemp)"
tmpfile2="$(mktemp)"
for method in ${methods[@]}; do

    inputfile="${statsfolder}/${method}-pthrput-${pthrput}-gthrput-20-gthreads-1-gsize-10.stderr"

    if [ ! -f $inputfile ]; then
        echo "ERROR: $inputfile does not exist!"
        continue
    fi

    # compute and print percentiles of latencies
    keep_get_stats $inputfile | sort -n > $tmpfile2
    average_using_sliding_window $tmpfile2
    cat $tmpfile2 | sort -k 3 -n > $tmpfile
    lines=`cat $tmpfile | wc -l`

    echo $method | awk '{printf "%-20s ", $1}'
    for p in ${percentiles[@]}; do
        if [ $p == "0" ]; then
            cat $tmpfile | awk '{s+=$3; n++} END{printf "%6.1f ", s/n}'
        else
            cat $tmpfile | awk -v p=$p -v lines=$lines 'NR >= p * lines {printf "%6.1f ", $3; exit}'
        fi
    done
    echo ""
done

rm $tmpfile $tmpfile2

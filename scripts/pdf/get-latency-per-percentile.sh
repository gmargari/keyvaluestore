#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh

statsfolder=$1

files=( 
"nomerge-putthrput-2500.stderr"
"cassandra-l-4-putthrput-2500.stderr"
"cassandra-l-3-putthrput-2500.stderr"
"cassandra-l-2-putthrput-2500.stderr"
"geometric-r-2-putthrput-2500.stderr"
"geometric-r-3-putthrput-2500.stderr"
"geometric-r-4-putthrput-2500.stderr"
"geometric-p-4-putthrput-2500.stderr"
"geometric-p-3-putthrput-2500.stderr"
"geometric-p-2-putthrput-2500.stderr"
"rangemerge-putthrput-2500.stderr"
"immediate-putthrput-2500.stderr"
)

# percentile = 0 -> print average time
percentiles=( "0" "0.90" "0.99" "0.999" "1" )

# print header
echo ""
echo "" | awk '{printf "%-20s ", $0}'
for p in ${percentiles[@]}; do
    if [ $p == "0" ]; then
        echo "avg" | awk '{printf "%5s ", $1}'
    else
        echo $p | awk '{printf "%5.1f ", $1*100}'
    fi
done 
echo -e "\n"

# print: method - get latencies
for file in ${files[@]}; do

    inputfile="${statsfolder}/$file"

    if [ ! -f $inputfile ]; then
        echo "ERROR: $inputfile does not exist!"
        continue
    fi

    method=`echo $file | awk '{split($1,a,"-putthrput"); print a[1]}'`

    getstats_file="$(mktemp)"
    keep_get_stats $inputfile > $getstats_file

    # compute and print percentiles of latencies
    tmpfile="$(mktemp)"
    cat $getstats_file | awk '{print $3}' | sort -n > $tmpfile
    lines=`cat $tmpfile | wc -l`

    echo $method | awk '{printf "%-20s ", $1}'
    for p in ${percentiles[@]}; do
        if [ $p == "0" ]; then
            cat $tmpfile | awk '{s+=$1;n++} END{printf "%5.1f ", s/n}'
        else
            cat $tmpfile | awk -v p=$p -v lines=$lines 'NR >= p * lines {printf "%5.1f ", $1; exit}'
        fi
    done
    echo ""

done

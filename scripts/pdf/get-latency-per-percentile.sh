#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1

files=( 
"nomerge-put-thrput-2500-mergebuf-512KB.stderr"
"cassandra-l-4-put-thrput-2500-mergebuf-512KB.stderr"
"cassandra-l-3-put-thrput-2500-mergebuf-512KB.stderr"
"cassandra-l-2-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-r-2-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-r-3-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-r-4-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-p-4-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-p-3-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-p-2-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-put-thrput-2500-mergebuf-512KB.stderr"
"immediate-put-thrput-2500-mergebuf-512KB.stderr"
)

# percentile = 0 -> print average time
percentiles=( "0" "0.50" "0.90" "0.99" "0.999" "1" )

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

    method=`echo $file | awk '{split($1,a,"-put-"); print a[1]}'`

    get_stats_file="$(mktemp)"
    tmpfile="$(mktemp)"

    # get_stats rows: "[GET_STATS] <timestamp> <time_elapsed> <num_requests> <totallatency>
    cat $inputfile | awk '{if ($1=="[GET_STATS]") {print $2, ($3 ? $4/$3 : 0), ($4 ? $5/$4 : 0)}}' > $get_stats_file

    # compute and print percentiles of latencies
    cat $get_stats_file | awk '{print $3}' | sort -n > $tmpfile
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

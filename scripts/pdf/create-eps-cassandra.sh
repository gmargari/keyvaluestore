#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

#nodes_1_client='workload-10GB-500rps-4threads.node30.node10.out'
nodes_1_client='workload-10GB-0500rps-8threads.node30.node4.out'
nodes_1_client_1000rps='workload-10GB-1000rps-4threads.node30.node10.out'
nodes_1_client_2500rps='workload-10GB-2500rps-4threads.node30.node10.out'
nodes_4_client='workload-40GB-2000rps-8threads.node30.node4,node9,node12,node14.out'
nodes_8_client='workload-80GB-4000rps-8threads.node30.node4,node7,node9,node10,node12,node13,node14,node15.out'
#nodes_1_server='workload-10GB-500rps-4threads.node30.node10.casslog'
nodes_1_server='workload-10GB-0500rps-8threads.node30.node4.casslog'
nodes_4_server='workload-40GB-2000rps-8threads.node30.node4,node9,node12,node14.node12.casslog'
nodes_8_server='workload-80GB-4000rps-8threads.node30.node4,node7,node9,node10,node12,node13,node14,node15.node4.casslog'

windowsizes=( 1 12 )

#========================================================
# average_using_sliding_window()
#========================================================
function average_using_sliding_window {
    local tmp="$(mktemp)"
    cat $1 | awk -v w=$windowsize \
      '{sumt += $2 - t[w]; \
        for (i = w; i >= 2; i--) {t[i] = t[i-1];} t[1] = $2;
        if (NR >= w) print $1, (sumt>0 ? sumt/w : 0)}' > $tmp
    mv $tmp $1
}

tmpfile=$(mktemp)

my_print



#======================================================
# bar chart with latency break down
#======================================================

file_nodes_1_client=${statsfolder}/cassandra-ycsb/${nodes_1_client}
file_nodes_4_client=${statsfolder}/cassandra-ycsb/${nodes_4_client}
file_nodes_8_client=${statsfolder}/cassandra-ycsb/${nodes_8_client}
file_nodes_1_server=${statsfolder}/cassandra-ycsb/${nodes_1_server}
file_nodes_4_server=${statsfolder}/cassandra-ycsb/${nodes_4_server}
file_nodes_8_server=${statsfolder}/cassandra-ycsb/${nodes_8_server}
outputfile=${outfolder}/cassandra.latency.breakdown.eps

ensure_file_exist ${file_nodes_1_client} ${file_nodes_4_client} ${file_nodes_8_client} 
ensure_file_exist ${file_nodes_1_server} ${file_nodes_4_server} ${file_nodes_8_server}

nodes_1_client_avg=`cat ${file_nodes_1_client} | awk '$1=="[get_range_slices]" {s+=$2;n++} END{print s/n}'`
nodes_1_server_avg=`cat ${file_nodes_1_server} | awk '$8=="[executeLocallyRangeSlice]" {s+=$9;n++} END{print s/n}'`
nodes_4_client_avg=`cat ${file_nodes_4_client} | awk '$1=="[get_range_slices]" {s+=$2;n++} END{print s/n}'`
nodes_4_server_avg=`cat ${file_nodes_4_server} | awk '$8=="[executeLocallyRangeSlice]" {s+=$9;n++} END{print s/n}'`
nodes_8_client_avg=`cat ${file_nodes_8_client} | awk '$1=="[get_range_slices]" {s+=$2;n++} END{print s/n}'`
nodes_8_server_avg=`cat ${file_nodes_8_server} | awk '$8=="[executeLocallyRangeSlice]" {s+=$9;n++} END{print s/n}'`

echo "1 ${nodes_1_server_avg} ${nodes_1_client_avg}" > ${tmpfile}
echo "2 ${nodes_4_server_avg} ${nodes_4_client_avg}" >> ${tmpfile}
echo "3 ${nodes_8_server_avg} ${nodes_8_client_avg}" >> ${tmpfile}

xticklabels='"1 server" 0, "4 servers" 1, "8 servers" 2';
yticklabels='"100%%" 1, "80%%" 0.8, "60%%" 0.6, "40%%" 0.4, "20%%" 0.2';
ymax='1.3'

my_print "cassandra latency breakdown"

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    set terminal postscript color enhanced eps "Helvetica" 24
    set xtics (${xticklabels}) # rotate by -45
    set ylabel 'Range query latency (ms)'

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms

    set size 1.0,0.7

    set key invert
    set key top right

    set style line 1 lt 1 lw 2 lc rgb '${color}'
    set style line 2 lt 1 lw 2 lc rgb '${color2}'

    set out '${outputfile}'
    plot \
    '${tmpfile}' using (\$2)       ls 1 title 'server (storage)', \
    '${tmpfile}' using (\$3 - \$2) ls 2 title 'server (indexing) + client' 

EOF
#==========================[ gnuplot embedded script ]============================



#======================================================
# range get latency timeline 
#======================================================

for file in $nodes_1_client $nodes_1_client_1000rps $nodes_1_client_2500rps $nodes_4_client $nodes_8_client; do
    
    for windowsize in ${windowsizes[@]}; do

        inputfile=${statsfolder}/cassandra-ycsb/${file}
        outputfile=${outfolder}/cassandra.${file}.${windowsize}.eps

        my_print ${file} window = ${windowsize}
        ensure_file_exist ${inputfile}
        cat ${inputfile} | awk '$1=="[SCAN]," {if (++i > 6) printf "%.2f %f\n", $2/60000, $3/1000}' > ${tmpfile} # first 5 lines contain avg stats
        average_using_sliding_window ${tmpfile}

        xmin=-2
        xmax=`cat ${tmpfile} | awk '{if ($1 > max) max=$1;} END{print max+2}'`
        ymax=`cat ${tmpfile} | awk '{if ($2 > max) max=$2;} END{print max * 1.05}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    set terminal postscript color enhanced eps "Helvetica" 24
    set xlabel '${xlabel_time}'
    set xrange [${xmin}:${xmax}]
    set yrange [0:${ymax}]

    set size 1.0,0.7

    # legend position
    set key top left

    set ylabel 'Range query latency (ms)'

    set out '${outputfile}'
    plot '${tmpfile}' using 1:2 with lines lw 4 lc rgb '${color}' notitle

EOF
#==========================[ gnuplot embedded script ]============================

    done
done

rm ${tmpfile}


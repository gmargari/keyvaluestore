#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1
outfolder="$statsfolder/eps"
mkdir -p $outfolder

#========================================================
# my_print()
#========================================================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

#========================================================
# average_using_sliding_window()
#========================================================
function average_using_sliding_window {
    tmp="$(mktemp)"
    cat $1 | awk -v w=$windowsize \
      '{sumt += $2 - t[w]; suml += $3 - l[w]; \
        for (i = w; i >= 2; i--) {t[i] = t[i-1]; l[i] = l[i-1];} t[1] = $2; l[1] = $3; 
        if (NR >= w) print $1, (sumt>0 ? sumt/w : 0), (suml>0 ? suml/w : 0)}' > $tmp
    mv $tmp $1
}

#========================================================
# main script starts here
#========================================================

# average using a sliding of size 'windowsize'
windowsize=15
color="#387DB8"

files=( 
"geometric-r-2-put-thrput-0-mergebuf-512KB.stderr"
"geometric-r-3-put-thrput-0-mergebuf-512KB.stderr"
"geometric-r-4-put-thrput-0-mergebuf-512KB.stderr"
"cassandra-l-2-put-thrput-0-mergebuf-512KB.stderr"
"cassandra-l-3-put-thrput-0-mergebuf-512KB.stderr"
"cassandra-l-4-put-thrput-0-mergebuf-512KB.stderr"
"geometric-p-2-put-thrput-0-mergebuf-512KB.stderr"
"geometric-p-3-put-thrput-0-mergebuf-512KB.stderr"
"geometric-p-4-put-thrput-0-mergebuf-512KB.stderr"
"immediate-put-thrput-0-mergebuf-512KB.stderr"
"nomerge-put-thrput-0-mergebuf-512KB.stderr"
"rangemerge-put-thrput-0-mergebuf-512KB.stderr"
\
"geometric-r-2-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-r-3-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-r-4-put-thrput-2500-mergebuf-512KB.stderr"
"cassandra-l-2-put-thrput-2500-mergebuf-512KB.stderr"
"cassandra-l-3-put-thrput-2500-mergebuf-512KB.stderr"
"cassandra-l-4-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-p-2-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-p-3-put-thrput-2500-mergebuf-512KB.stderr"
"geometric-p-4-put-thrput-2500-mergebuf-512KB.stderr"
"immediate-put-thrput-2500-mergebuf-512KB.stderr"
"nomerge-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-put-thrput-2500-mergebuf-512KB.stderr"
\
"geometric-r-2-put-thrput-0-mergebuf-4MB.stderr"
"geometric-r-3-put-thrput-0-mergebuf-4MB.stderr"
"geometric-r-4-put-thrput-0-mergebuf-4MB.stderr"
"cassandra-l-2-put-thrput-0-mergebuf-4MB.stderr"
"cassandra-l-3-put-thrput-0-mergebuf-4MB.stderr"
"cassandra-l-4-put-thrput-0-mergebuf-4MB.stderr"
"geometric-p-2-put-thrput-0-mergebuf-4MB.stderr"
"geometric-p-3-put-thrput-0-mergebuf-4MB.stderr"
"geometric-p-4-put-thrput-0-mergebuf-4MB.stderr"
"immediate-put-thrput-0-mergebuf-4MB.stderr"
"nomerge-put-thrput-0-mergebuf-4MB.stderr"
"rangemerge-put-thrput-0-mergebuf-4MB.stderr"
\
"geometric-r-2-put-thrput-2500-mergebuf-4MB.stderr"
"geometric-r-3-put-thrput-2500-mergebuf-4MB.stderr"
"geometric-r-4-put-thrput-2500-mergebuf-4MB.stderr"
"cassandra-l-2-put-thrput-2500-mergebuf-4MB.stderr"
"cassandra-l-3-put-thrput-2500-mergebuf-4MB.stderr"
"cassandra-l-4-put-thrput-2500-mergebuf-4MB.stderr"
"geometric-p-2-put-thrput-2500-mergebuf-4MB.stderr"
"geometric-p-3-put-thrput-2500-mergebuf-4MB.stderr"
"geometric-p-4-put-thrput-2500-mergebuf-4MB.stderr"
"immediate-put-thrput-2500-mergebuf-4MB.stderr"
"nomerge-put-thrput-2500-mergebuf-4MB.stderr"
"rangemerge-put-thrput-2500-mergebuf-4MB.stderr"
\
"rangemerge-blocksize-0016-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0032-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0064-put-thrput-2500-mergebuf-512KB.stderr"
#"rangemerge-blocksize-0128-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0256-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0512-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-1024-put-thrput-2500-mergebuf-512KB.stderr"
)

for file in ${files[@]}; do

    inputfile="${statsfolder}/$file"
    outputfile="`echo ${outfolder}/$file | awk '{sub(/\\.stderr/,\"\", $1); print $1}'`"  # remove suffix .stderr

    if [ ! -f $inputfile ]; then
        echo "ERROR: $inputfile does not exist!"
        continue
    fi

    method=`echo $file | awk '{split($1,a,"-put-"); print a[1]}'`

    my_print $method

    get_stats_file="$(mktemp)"
    flush_stats_file="$(mktemp)"
    flush_stats_file2="$(mktemp)"
    tmpfile="$(mktemp)"

    # get_stats rows: "[GET_STATS] <timestamp> <time_elapsed> <num_requests> <totallatency>
    cat $inputfile | awk '{if ($1=="[GET_STATS]") {print $2, ($3 ? $4/$3 : 0), ($4 ? $5/$4 : 0)}}' > $get_stats_file
    cat $inputfile | awk '{if ($1=="[FLUSH_MEM]") {print $2, $3, $4}}' > $flush_stats_file

    # take average using a sliding window
    average_using_sliding_window $get_stats_file

    thrput_min=`cat $get_stats_file | awk 'NR >= 100 {if (NR==100) {min=$2} else if ($2 < min) {min=$2}} END{print min}'`
    thrput_max=`cat $get_stats_file | awk '$2 > max {max=$2} END{print max}'`
    latency_max=`cat $get_stats_file | awk '$3 > max {max=$3} END{print max}'`
    time_max=`cat $get_stats_file | awk '{max=$1} END{print max}'`

    # 'flush_stats_file2' is used to draw bars during compactions
    cat $flush_stats_file | awk '{if ($2=="START") {print $1, $2, 0; print $1, $2, 1;} else {print $1, $2, 1; print $1, $2, 0;}}' > $flush_stats_file2

#########################################
# MOVE IN SEPARATE FILE
#########################################
#    # compute and print percentiles of latencies (not used for plots, just for info)
#    cat $get_stats_file | awk '{print $3}' | sort -n > $tmpfile
#    lines=`cat $tmpfile | wc -l`
#    for p in 0.50 0.90 0.99 0.999 1; do
#        cat $tmpfile | awk -v p=$p -v lines=$lines 'NR >= p * lines {printf "latency %3.1f%% %5.1f\n", p*100, $1; exit}'
#    done
#    echo min throughput = $thrput_min
#########################################


#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 30
    set xlabel 'Time (min)'
    set xrange [0:(${time_max}/60000)*1.05]

#    set size ratio 0.4

    ms_to_min(x) = x/60000
    const_height_thrput(x) = (x != 0) ? $thrput_max : 0
    const_height_latency(x) = (x != 0) ? $latency_max : 0

    set out '${outputfile}.latency.eps'
    set ylabel 'Get Latency (ms)'
    set yrange [0:${latency_max}*1.05]
    plot \
    '${flush_stats_file2}' using (ms_to_min(\$1)):(const_height_latency(\$3)) with filledcurve lc rgb "#eeeeee" notitle, \
    '${get_stats_file}'    using (ms_to_min(\$1)):3 with lines lw 3 lt 1 lc rgb '$color' notitle

    set out '${outputfile}.thrput.eps'
    set ylabel 'Get throughput (req/s)'
    set yrange [0:${thrput_max}*1.05]
    plot \
    '${flush_stats_file2}' using (ms_to_min(\$1)):(const_height_thrput(\$3)) with filledcurve lc rgb "#eeeeee" notitle, \
    '${get_stats_file}'    using (ms_to_min(\$1)):2 with lines lw 3 lt 1 lc rgb '$color' notitle

EOF
#==========================[ gnuplot embedded script ]============================

    rm $get_stats_file $flush_stats_file $flush_stats_file2 $tmpfile

done

#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

files=(
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"nomerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
\
"geometric-r-2-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-4-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-2-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-3-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-3-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-4-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"nomerge-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-0-gthrput-20-gthreads-1-gsize-10.stderr"
\
# Put throughput
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-1.stderr"
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-100.stderr"
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-1000.stderr"
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10000.stderr"
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-100000.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-1.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-100.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-1000.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10000.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-100000.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-1.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-100.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-1000.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10000.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-100000.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-1.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-100.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-1000.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10000.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-100000.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-1.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-100.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-1000.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-10000.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-100000.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-1.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-100.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-1000.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10000.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-100000.stderr"
\
# Number of get threads
#"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"geometric-r-2-pthrput-2500-gthrput-10-gthreads-2-gsize-10.stderr"
#"geometric-r-2-pthrput-2500-gthrput-4-gthreads-5-gsize-10.stderr"
#"geometric-r-2-pthrput-2500-gthrput-2-gthreads-10-gsize-10.stderr"
#"geometric-r-2-pthrput-2500-gthrput-1-gthreads-20-gsize-10.stderr"
#"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"cassandra-l-4-pthrput-2500-gthrput-10-gthreads-2-gsize-10.stderr"
#"cassandra-l-4-pthrput-2500-gthrput-4-gthreads-5-gsize-10.stderr"
#"cassandra-l-4-pthrput-2500-gthrput-2-gthreads-10-gsize-10.stderr"
#"cassandra-l-4-pthrput-2500-gthrput-1-gthreads-20-gsize-10.stderr"
#"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"geometric-r-3-pthrput-2500-gthrput-10-gthreads-2-gsize-10.stderr"
#"geometric-r-3-pthrput-2500-gthrput-4-gthreads-5-gsize-10.stderr"
#"geometric-r-3-pthrput-2500-gthrput-2-gthreads-10-gsize-10.stderr"
#"geometric-r-3-pthrput-2500-gthrput-1-gthreads-20-gsize-10.stderr"
#"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"rangemerge-pthrput-2500-gthrput-10-gthreads-2-gsize-10.stderr"
#"rangemerge-pthrput-2500-gthrput-4-gthreads-5-gsize-10.stderr"
#"rangemerge-pthrput-2500-gthrput-2-gthreads-10-gsize-10.stderr"
#"rangemerge-pthrput-2500-gthrput-1-gthreads-20-gsize-10.stderr"
#"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"immediate-pthrput-2500-gthrput-10-gthreads-2-gsize-10.stderr"
#"immediate-pthrput-2500-gthrput-4-gthreads-5-gsize-10.stderr"
#"immediate-pthrput-2500-gthrput-2-gthreads-10-gsize-10.stderr"
#"immediate-pthrput-2500-gthrput-1-gthreads-20-gsize-10.stderr"
#"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
#"geometric-p-2-pthrput-2500-gthrput-10-gthreads-2-gsize-10.stderr"
#"geometric-p-2-pthrput-2500-gthrput-4-gthreads-5-gsize-10.stderr"
#"geometric-p-2-pthrput-2500-gthrput-2-gthreads-10-gsize-10.stderr"
#"geometric-p-2-pthrput-2500-gthrput-1-gthreads-20-gsize-10.stderr"
\
# Get size 
"geometric-r-2-pthrput-1000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-2-pthrput-5000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-2-pthrput-10000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-2-pthrput-20000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-2-pthrput-40000-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-1000-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-5000-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-10000-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-20000-gthrput-20-gthreads-1-gsize-10.stderr"
"cassandra-l-4-pthrput-40000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-1000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-5000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-10000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-20000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-r-3-pthrput-40000-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-1000-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-5000-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-10000-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-20000-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-pthrput-40000-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-1000-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-5000-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-10000-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-20000-gthrput-20-gthreads-1-gsize-10.stderr"
"immediate-pthrput-40000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-1000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-5000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-10000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-20000-gthrput-20-gthreads-1-gsize-10.stderr"
"geometric-p-2-pthrput-40000-gthrput-20-gthreads-1-gsize-10.stderr"
\
"rangemerge-b-0032-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-0064-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-0128-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-0256-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-0512-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-1024-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-2048-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
"rangemerge-b-0000-pthrput-2500-gthrput-20-gthreads-1-gsize-10.stderr"
)

for file in ${files[@]}; do

    inputfile="${statsfolder}/$file"
    outputfile="`echo ${outfolder}/$file | awk '{sub(/\\.stderr/,\"\", $1); print $1}'`"  # remove suffix .stderr

    method=`echo $file | awk '{split($1,a,"-put-"); print a[1]}'`
    my_print $method
    ensure_file_exist $inputfile

    getstats_file="$(mktemp)"
    flushstats_file="$(mktemp)"
    flushstats_file2="$(mktemp)"
    tmpfile="$(mktemp)"

    keep_get_stats $inputfile > $getstats_file
    keep_flush_stats $inputfile > $flushstats_file
    average_using_sliding_window $getstats_file

    thrput_max=`cat $getstats_file  | awk '$2 > max {max=$2} END{print max}'`
    latency_max=`cat $getstats_file | awk '$3 > max {max=$3} END{print max}'`
    time_max=`cat $getstats_file    | awk '$1 > max {max=$1} END{print max}'`

    # 'flushstats_file2' is used to draw bars during compactions
    cat $flushstats_file | awk '{if ($2=="START") {print $1, $2, 0; print $1, $2, 1;} else {print $1, $2, 1; print $1, $2, 0;}}' > $flushstats_file2

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
    set xlabel font 'Helvetica,25'
    set ylabel font 'Helvetica,25'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_time}'
    set xrange [0:(${time_max}/60000)*1.05]

    ms_to_min(x) = x/60000
    const_height_thrput(x) = (x != 0) ? $thrput_max : 0
    const_height_latency(x) = (x != 0) ? $latency_max : 0

    set size noratio

    #---------------------------------------
    # eps for get latency
    #---------------------------------------

    set out '${outputfile}.latency.eps'
    set ylabel '${ylabel_glatency}'
    set yrange [0:${latency_max}*1.05]
    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_latency(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):3 with lines lw 3 lt 1 lc rgb '${color}' notitle


    #---------------------------------------
    # eps for get throughput
    #---------------------------------------

    set out '${outputfile}.thrput.eps'
    set ylabel '${ylabel_gthrput}'
    set yrange [0:${thrput_max}*1.05]
    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_thrput(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):2 with lines lw 3 lt 1 lc rgb '${color}' notitle


    #---------------------------------------
    # get latency and throughput in same eps
    #---------------------------------------

    set size ratio 0.3

    set out '${outputfile}.multiple.latency.eps'
    set multiplot

    set ylabel '${ylabel_glatency}'
    set xlabel ''
    set yrange [0:${latency_max}*1.05]
#    set xtics 20
    unset xtics

    set size 1,0.6
    set origin 0.0,0.5
    set bmargin 0
    set tmargin 1

    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_latency(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):3 with lines lw 3 lt 1 lc rgb '${color}' notitle

    set ylabel '${ylabel_gthrput}'
    set xlabel '${xlabel_time}'
    set yrange [0:${thrput_max}*1.05]
    set xtics 20

    set size 1,0.6
    set origin 0.0,0.05
    set bmargin 0
    set tmargin 1

    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_thrput(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):2 with lines lw 3 lt 1 lc rgb '${color}' notitle

    set nomultiplot


EOF
#==========================[ gnuplot embedded script ]============================

    rm $getstats_file $flushstats_file $flushstats_file2 $tmpfile

done

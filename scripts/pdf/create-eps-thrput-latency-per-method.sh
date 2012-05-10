#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

files=( 
"geometric-r-2-putthrput-0.stderr"
"geometric-r-3-putthrput-0.stderr"
"geometric-r-4-putthrput-0.stderr"
"cassandra-l-2-putthrput-0.stderr"
"cassandra-l-3-putthrput-0.stderr"
"cassandra-l-4-putthrput-0.stderr"
"geometric-p-2-putthrput-0.stderr"
"geometric-p-3-putthrput-0.stderr"
"geometric-p-4-putthrput-0.stderr"
"immediate-putthrput-0.stderr"
"nomerge-putthrput-0.stderr"
"rangemerge-putthrput-0.stderr"
\
"geometric-r-2-full-putthrput-full-getthrput.stderr"
"geometric-r-3-full-putthrput-full-getthrput.stderr"
"geometric-r-4-full-putthrput-full-getthrput.stderr"
"cassandra-l-2-full-putthrput-full-getthrput.stderr"
"cassandra-l-3-full-putthrput-full-getthrput.stderr"
"cassandra-l-4-full-putthrput-full-getthrput.stderr"
"geometric-p-2-full-putthrput-full-getthrput.stderr"
"geometric-p-3-full-putthrput-full-getthrput.stderr"
"geometric-p-4-full-putthrput-full-getthrput.stderr"
"immediate-full-putthrput-full-getthrput.stderr"
"nomerge-full-putthrput-full-getthrput.stderr"
"rangemerge-full-putthrput-full-getthrput.stderr"
\
"geometric-r-2-putthrput-2500.stderr"
"geometric-r-3-putthrput-2500.stderr"
"geometric-r-4-putthrput-2500.stderr"
"cassandra-l-2-putthrput-2500.stderr"
"cassandra-l-3-putthrput-2500.stderr"
"cassandra-l-4-putthrput-2500.stderr"
"geometric-p-2-putthrput-2500.stderr"
"geometric-p-3-putthrput-2500.stderr"
"geometric-p-4-putthrput-2500.stderr"
"immediate-putthrput-2500.stderr"
"nomerge-putthrput-2500.stderr"
"rangemerge-putthrput-2500.stderr"
\
"rangemerge-blocksize-0016-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0032-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0064-put-thrput-2500-mergebuf-512KB.stderr"
#"rangemerge-blocksize-0128-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0256-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-0512-put-thrput-2500-mergebuf-512KB.stderr"
"rangemerge-blocksize-1024-put-thrput-2500-mergebuf-512KB.stderr"
\
"logarithmic-getthreads-1-getthrput-20-getsize-1.stderr"
"logarithmic-getthreads-1-getthrput-20-getsize-10.stderr"
"logarithmic-getthreads-1-getthrput-20-getsize-100.stderr"
"logarithmic-getthreads-1-getthrput-20-getsize-1000.stderr"
"logarithmic-getthreads-1-getthrput-20-getsize-10000.stderr"
"logarithmic-getthreads-1-getthrput-20-getsize-100000.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-1.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-10.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-100.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-1000.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-10000.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-100000.stderr"
"geometric-getthreads-1-getthrput-20-getsize-1.stderr"
"geometric-getthreads-1-getthrput-20-getsize-10.stderr"
"geometric-getthreads-1-getthrput-20-getsize-100.stderr"
"geometric-getthreads-1-getthrput-20-getsize-1000.stderr"
"geometric-getthreads-1-getthrput-20-getsize-10000.stderr"
"geometric-getthreads-1-getthrput-20-getsize-100000.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-1.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-10.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-100.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-1000.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-10000.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-100000.stderr"
"immediate-getthreads-1-getthrput-20-getsize-1.stderr"
"immediate-getthreads-1-getthrput-20-getsize-10.stderr"
"immediate-getthreads-1-getthrput-20-getsize-100.stderr"
"immediate-getthreads-1-getthrput-20-getsize-1000.stderr"
"immediate-getthreads-1-getthrput-20-getsize-10000.stderr"
"immediate-getthreads-1-getthrput-20-getsize-100000.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-1.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-10.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-100.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-1000.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-10000.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-100000.stderr"
\
"logarithmic-getthreads-1-getthrput-20-getsize-10.stderr"
"logarithmic-getthreads-2-getthrput-10-getsize-10.stderr"
"logarithmic-getthreads-5-getthrput-4-getsize-10.stderr"
"logarithmic-getthreads-10-getthrput-2-getsize-10.stderr"
"logarithmic-getthreads-20-getthrput-1-getsize-10.stderr"
"cassandra-getthreads-1-getthrput-20-getsize-10.stderr"
"cassandra-getthreads-2-getthrput-10-getsize-10.stderr"
"cassandra-getthreads-5-getthrput-4-getsize-10.stderr"
"cassandra-getthreads-10-getthrput-2-getsize-10.stderr"
"cassandra-getthreads-20-getthrput-1-getsize-10.stderr"
"geometric-getthreads-1-getthrput-20-getsize-10.stderr"
"geometric-getthreads-2-getthrput-10-getsize-10.stderr"
"geometric-getthreads-5-getthrput-4-getsize-10.stderr"
"geometric-getthreads-10-getthrput-2-getsize-10.stderr"
"geometric-getthreads-20-getthrput-1-getsize-10.stderr"
"rangemerge-getthreads-1-getthrput-20-getsize-10.stderr"
"rangemerge-getthreads-2-getthrput-10-getsize-10.stderr"
"rangemerge-getthreads-5-getthrput-4-getsize-10.stderr"
"rangemerge-getthreads-10-getthrput-2-getsize-10.stderr"
"rangemerge-getthreads-20-getthrput-1-getsize-10.stderr"
"immediate-getthreads-1-getthrput-20-getsize-10.stderr"
"immediate-getthreads-2-getthrput-10-getsize-10.stderr"
"immediate-getthreads-5-getthrput-4-getsize-10.stderr"
"immediate-getthreads-10-getthrput-2-getsize-10.stderr"
"immediate-getthreads-20-getthrput-1-getsize-10.stderr"
"geometric-p-2-getthreads-1-getthrput-20-getsize-10.stderr"
"geometric-p-2-getthreads-2-getthrput-10-getsize-10.stderr"
"geometric-p-2-getthreads-5-getthrput-4-getsize-10.stderr"
"geometric-p-2-getthreads-10-getthrput-2-getsize-10.stderr"
"geometric-p-2-getthreads-20-getthrput-1-getsize-10.stderr"
\
"logarithmic-putthrput-1000.stderr"
"logarithmic-putthrput-2500.stderr"
"logarithmic-putthrput-5000.stderr"
"logarithmic-putthrput-10000.stderr"
"logarithmic-putthrput-20000.stderr"
"logarithmic-putthrput-40000.stderr"
"cassandra-putthrput-1000.stderr"
"cassandra-putthrput-2500.stderr"
"cassandra-putthrput-5000.stderr"
"cassandra-putthrput-10000.stderr"
"cassandra-putthrput-20000.stderr"
"cassandra-putthrput-40000.stderr"
"geometric-putthrput-1000.stderr"
"geometric-putthrput-2500.stderr"
"geometric-putthrput-5000.stderr"
"geometric-putthrput-10000.stderr"
"geometric-putthrput-20000.stderr"
"geometric-putthrput-40000.stderr"
"rangemerge-putthrput-1000.stderr"
"rangemerge-putthrput-2500.stderr"
"rangemerge-putthrput-5000.stderr"
"rangemerge-putthrput-10000.stderr"
"rangemerge-putthrput-20000.stderr"
"rangemerge-putthrput-40000.stderr"
"immediate-putthrput-1000.stderr"
"immediate-putthrput-2500.stderr"
"immediate-putthrput-5000.stderr"
"immediate-putthrput-10000.stderr"
"immediate-putthrput-20000.stderr"
"immediate-putthrput-40000.stderr"
"geometric-p-2-putthrput-1000.stderr"
"geometric-p-2-putthrput-2500.stderr"
"geometric-p-2-putthrput-5000.stderr"
"geometric-p-2-putthrput-10000.stderr"
"geometric-p-2-putthrput-20000.stderr"
"geometric-p-2-putthrput-40000.stderr"
)

for file in ${files[@]}; do

    inputfile="${statsfolder}/$file"
    outputfile="`echo ${outfolder}/$file | awk '{sub(/\\.stderr/,\"\", $1); print $1}'`"  # remove suffix .stderr

    ensure_file_exist $inputfile

    method=`echo $file | awk '{split($1,a,"-put-"); print a[1]}'`

    my_print $method

    getstats_file="$(mktemp)"
    flushstats_file="$(mktemp)"
    flushstats_file2="$(mktemp)"
    tmpfile="$(mktemp)"

    keep_get_stats $inputfile > $getstats_file
    keep_flush_stats $inputfile > $flushstats_file
    average_using_sliding_window $getstats_file

    thrput_min=`cat $getstats_file  | awk 'NR >= 100 {if (NR==100) {min=$2} else if ($2 < min) {min=$2}} END{print min}'`
    thrput_max=`cat $getstats_file  | awk '$2 > max {max=$2} END{print max}'`
    latency_max=`cat $getstats_file | awk '$3 > max {max=$3} END{print max}'`
    time_max=`cat $getstats_file    | awk '$1 > max {max=$1} END{print max}'`

    # 'flushstats_file2' is used to draw bars during compactions
    cat $flushstats_file | awk '{if ($2=="START") {print $1, $2, 0; print $1, $2, 1;} else {print $1, $2, 1; print $1, $2, 0;}}' > $flushstats_file2

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel 'Time (min)'
    set xrange [0:(${time_max}/60000)*1.05]

    ms_to_min(x) = x/60000
    const_height_thrput(x) = (x != 0) ? $thrput_max : 0
    const_height_latency(x) = (x != 0) ? $latency_max : 0

    set size noratio

    #---------------------------------------
    # eps for get latency
    #---------------------------------------

    set out '${outputfile}.latency.eps'
    set ylabel 'Get Latency (ms)'
    set yrange [0:${latency_max}*1.05]
    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_latency(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):3 with lines lw 3 lt 1 lc rgb '$color' notitle


    #---------------------------------------
    # eps for get throughput
    #---------------------------------------

    set out '${outputfile}.thrput.eps'
    set ylabel 'Get throughput (req/s)'
    set yrange [0:${thrput_max}*1.05]
    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_thrput(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):2 with lines lw 3 lt 1 lc rgb '$color' notitle


    #---------------------------------------
    # get latency and throughput in same eps
    #---------------------------------------

    set size ratio 0.3

    set out '${outputfile}.multiple.latency.eps'
    set multiplot

    set ylabel 'Get Latency (ms)'
    set xlabel ''
    set yrange [0:${latency_max}*1.05]
    set xtics 20

    set size 1,0.6
    set origin 0.0,0.5
    set bmargin 0
    set tmargin 1

    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_latency(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):3 with lines lw 3 lt 1 lc rgb '$color' notitle

    set ylabel 'Get throughput (req/s)'
    set xlabel 'Time (min)'
    set yrange [0:${thrput_max}*1.05]
    set xtics 20

    set size 1,0.6
    set origin 0.0,0.05
    set bmargin 0
    set tmargin 1

    plot \
    '${flushstats_file2}' using (ms_to_min(\$1)):(const_height_thrput(\$3)) with filledcurve lc rgb '$bgmerge_color' notitle, \
    '${getstats_file}'    using (ms_to_min(\$1)):2 with lines lw 3 lt 1 lc rgb '$color' notitle

    set nomultiplot


EOF
#==========================[ gnuplot embedded script ]============================

    rm $getstats_file $flushstats_file $flushstats_file2 $tmpfile

done

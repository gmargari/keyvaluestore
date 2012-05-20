#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

skip=5
putthrputs=( '1000' '2500' '5000' '10000' '20000' '40000' )

getstats_cas="$(mktemp)"
getstats_log="$(mktemp)"
getstats_geo="$(mktemp)"
getstats_rng="$(mktemp)"
getstats_gp2="$(mktemp)"

for putthrput in ${putthrputs[@]}; do

    file_cas="${statsfolder}/cassandra-l-4-pthrput-${putthrput}-gthrput-20-gthreads-1-gsize-10.stderr"
    file_log="${statsfolder}/geometric-r-2-pthrput-${putthrput}-gthrput-20-gthreads-1-gsize-10.stderr"
    file_geo="${statsfolder}/geometric-r-3-pthrput-${putthrput}-gthrput-20-gthreads-1-gsize-10.stderr"
    file_rng="${statsfolder}/rangemerge-pthrput-${putthrput}-gthrput-20-gthreads-1-gsize-10.stderr"
    file_gp2="${statsfolder}/geometric-p-2-pthrput-${putthrput}-gthrput-20-gthreads-1-gsize-10.stderr"

    # override styles
    style_log="lines lw 3 lt 4 lc rgb $color_log"
    style_geo="lines lw 5 lt 3 lc rgb $color_geo"
    style_rng="lines lw 4 lt 1 lc rgb $color_rng"
    style_gp2="lines lw 1 lt 5 lc rgb $color_gp2"
    style_cas="lines lw 2 lt 5 lc rgb $color_cas"
#    style_cas="lines lw 2 lt 5 lc rgb '#D9A621'"

    outputfile="${outfolder}/allmethods-pthrput-${putthrput}"

    my_print $putthrput
    ensure_file_exist $file_cas $file_log $file_geo $file_rng $file_gp2

    keep_get_stats $file_cas > $getstats_cas
    keep_get_stats $file_log > $getstats_log
    keep_get_stats $file_geo > $getstats_geo
    keep_get_stats $file_rng > $getstats_rng
    keep_get_stats $file_gp2 > $getstats_gp2

    average_using_sliding_window $getstats_cas
    average_using_sliding_window $getstats_log
    average_using_sliding_window $getstats_geo
    average_using_sliding_window $getstats_rng
    average_using_sliding_window $getstats_gp2

#    latency_max=`cat $getstats_log $getstats_geo $getstats_rng $getstats_gp2 | awk '$3 > max {max=$3} END{print max}'`
#    thrput_max=` cat $getstats_log $getstats_geo $getstats_rng $getstats_gp2 | awk '$2 > max {max=$2} END{print max}'`
#    time_max=`   cat $getstats_log $getstats_geo $getstats_rng $getstats_gp2 | awk '$1 > max {max=$1} END{print max}'`
    latency_max=`cat $getstats_cas $getstats_geo $getstats_rng | awk '$3 > max {max=$3} END{print max}'`
    thrput_max=` cat $getstats_cas $getstats_geo $getstats_rng | awk '$2 > max {max=$2} END{print max}'`
    time_max=`   cat $getstats_cas $getstats_geo $getstats_rng | awk '$1 > max {max=$1} END{print max}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel 'Time (min)'
    set xrange [0:(${time_max}/60000)*1.05]
    set key top left

    ms_to_min(x) = x/60000

    set out '${outputfile}.latency.eps'
    set ylabel 'Get latency (ms)'
    set yrange [0:${latency_max}*1.05]
    plot \
    '${getstats_cas}' using (ms_to_min(\$1)):3 every $skip with $style_cas title '$title_cas', \
    '${getstats_geo}' using (ms_to_min(\$1)):3 every $skip with $style_geo title '$title_geo', \
    '${getstats_rng}' using (ms_to_min(\$1)):3 every $skip with $style_rng title '$title_rng'
#    '${getstats_log}' using (ms_to_min(\$1)):3 every $skip with $style_log title '$title_log', \
#    '${getstats_gp2}' using (ms_to_min(\$1)):3 every $skip with $style_gp2 title '$title_gp2', \

    set key bottom right

    set out '${outputfile}.thrput.eps'
    set ylabel 'Get throughput (req/s)'
    set yrange [0:${thrput_max}*1.05]
    plot \
    '${getstats_cas}' using (ms_to_min(\$1)):2 every $skip with $style_cas title '$title_cas', \
    '${getstats_geo}' using (ms_to_min(\$1)):2 every $skip with $style_geo title '$title_geo', \
    '${getstats_rng}' using (ms_to_min(\$1)):2 every $skip with $style_rng title '$title_rng'
#    '${getstats_log}' using (ms_to_min(\$1)):2 every $skip with $style_log title '$title_log', \
#    '${getstats_gp2}' using (ms_to_min(\$1)):2 every $skip with $style_gp2 title '$title_gp2', \

EOF
#==========================[ gnuplot embedded script ]============================

    rm $getstats_cas $getstats_log $getstats_geo $getstats_rng $getstats_gp2 $tmpfile

done


#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

#==========================================================================================
# Logarithmic: Number of runs vs range get time
#==========================================================================================

file="geometric-r-2.searchlog"
inputfile="${statsfolder}/${file}"
outputfile="${outfolder}/${file}.eps"

my_print
ensure_file_exist ${inputfile}

tmpfile1="$(mktemp)"
tmpfile2="$(mktemp)"
cat $inputfile | awk '{if ($0 ~ "^ ") print $1, $15}' > $tmpfile1
cat $inputfile | awk '{if ($0 ~ "^ ") mbins=$1; if ($1 == "avg_rng_get_ms") print mbins, $2}' > $tmpfile2

ymax=`cat $tmpfile1 | awk '{if ($2 > max) max=$2;} END{print max}'`
y2max=`cat $tmpfile2 | awk '{if ($2 > max) max=$2;} END{print max}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,25'
#    set ytics font  'Helvetica,25'
#    set y2tics font  'Helvetica,25'
    set xlabel font 'Helvetica,25'
    set ylabel font 'Helvetica,25'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_datains_gb}'

set lmargin 8
set rmargin 7
set tmargin 1
set bmargin 3.2

    # legend position
    set key top right
#    set key at 5.2,4.7

    set yrange [0:${ymax}+1]
    set ylabel '${ylabel_runs}'
    set ytic nomirror
    set y2range [0:${y2max}+1]
#    set y2label '${ylabel_glatency}'
    set label '{/Helvetica=26 ${ylabel_glatency}}' at first  11.4, first  1.3 rotate by 90
    set y2tic nomirror

    mb2gb(x) = x/1024.0

    set out '${outputfile}'
    plot \
    '${tmpfile1}' using (mb2gb(\$1)):2                        with histeps lw 4 lc rgb '${color}' title '{/Helvetica=25 Number of disk files}' , \
    '${tmpfile2}' using (mb2gb(\$1)):((\$2 / $y2max) * $ymax) with histeps lw 4 lc rgb '${color3}' title '{/Helvetica=25 Get latency}'
#    plot \
#    '${tmpfile1}' using (mb2gb(\$1)):2                        with lines lw 4 lc rgb '${color}' title '{/Helvetica=25 Number of disk files}' , \
#    '${tmpfile2}' using (mb2gb(\$1)):((\$2 / $y2max) * $ymax) with lines lw 4 lc rgb '${color3}' title '{/Helvetica=25 Get latency}'

EOF
#==========================[ gnuplot embedded script ]============================

rm $tmpfile1 $tmpfile2


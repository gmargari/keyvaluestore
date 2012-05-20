#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

#--------------------------------------------------------------------------------
# You may need to "truncate" contents of files in order for the 2 plots
# to overlap better (e.g. if M1 = 128 MB, D1 = 2500 MB and M2 = 512 MB, keep
# from second file up to 10000 MB of data).
#--------------------------------------------------------------------------------

title1="M = 256MB, Data = 5GB"
title2="M = 2048MB, Data = 40GB"

rng_file1=${statsfolder}/rangemerge-memsize-0256-datasize-5000.log
rng_file2=${statsfolder}/rangemerge-memsize-2048-datasize-40000.log
rng_outputfile=${outfolder}/rangemerge-diff-mem-datasize-same-scale.eps

log_file1=${statsfolder}/geometric-r-2-memsize-0256-datasize-5000.log
log_file2=${statsfolder}/geometric-r-2-memsize-2048-datasize-40000.log
log_outputfile=${outfolder}/geometric-r-2-diff-mem-datasize-same-scale.eps

log_rng_outputfile=${outfolder}/geometric-r-2-rangemerge-diff-mem-datasize-same-scale.eps

ensure_file_exist $rng_file1 $rng_file2 $log_file1 $log_file2

rng_xmax1=`cat $rng_file1 | awk '{if ($1 > max) max=$1;} END{print max * 1.01}'`
rng_xmax2=`cat $rng_file2 | awk '{if ($1 > max) max=$1;} END{print max * 1.01}'`
rng_ymax1=`cat $rng_file1 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`
rng_ymax2=`cat $rng_file2 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`

log_xmax1=`cat $log_file1 | awk '{if ($1 > max) max=$1;} END{print max * 1.01}'`
log_xmax2=`cat $log_file2 | awk '{if ($1 > max) max=$1;} END{print max * 1.01}'`
log_ymax1=`cat $log_file1 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`
log_ymax2=`cat $log_file2 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`

my_print

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font   'Helvetica,21'
#    set ytics font   'Helvetica,21'
#    set y2tics font  'Helvetica,21'
#    set xlabel font  'Helvetica,22'
#    set ylabel font  'Helvetica,22'
#    set y2label font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 20
    set xlabel 'Data inserted (%)'

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    # legend position
    set key top left

    set ylabel 'Insertion time (min)'
    set y2label 'Insertion time (min)'
    set ytic nomirror
    set y2tic nomirror

    set size noratio

    #---------------------------------------
    # rangemerge 
    #---------------------------------------

    set out '${rng_outputfile}'
    set yrange [0:${rng_ymax1}+1]
    set y2range [0:${rng_ymax2}+1]

    plot \
   '$rng_file1' using (\$1 / $rng_xmax1 * 100):(sec2min(\$2))                             with lines lw 4 lc rgb '#E4191C' title "$title1", \
   '$rng_file2' using (\$1 / $rng_xmax2 * 100):(sec2min((\$2 / $rng_ymax2) * $rng_ymax1)) with lines lw 4 lc rgb '#387DB8' title "$title2"


    #---------------------------------------
    # logarithmic
    #---------------------------------------

    set out '${log_outputfile}'
    set yrange [0:${log_ymax1}+1]
    set y2range [0:${log_ymax2}+1]

    plot \
   '$log_file1' using (\$1 / $log_xmax1 * 100):(sec2min(\$2))                             with lines lw 4 lc rgb '#E4191C' title "$title1", \
   '$log_file2' using (\$1 / $log_xmax2 * 100):(sec2min((\$2 / $log_ymax2) * $log_ymax1)) with lines lw 4 lc rgb '#387DB8' title "$title2"


    #---------------------------------------
    # rangemerge and logarithmic in same eps
    #---------------------------------------

    set size ratio 0.3

    set out '${log_rng_outputfile}'

    set multiplot

    set ytics 5
    set y2tics 50
    set size 1,0.6
    set origin 0.0,0.5
    set bmargin 0
    set tmargin 1

    set xlabel 'Data inserted (%) - Geometric r=2'
    set yrange [0:15]
    set y2range [0:150]
    plot \
   '$log_file1' using (\$1 / $log_xmax1 * 100):(sec2min(\$2))                             with lines lw 4 lc rgb '#E4191C' title "$title1", \
   '$log_file2' using (\$1 / $log_xmax2 * 100):(sec2min((\$2 / $log_ymax2) * $log_ymax1)) with lines lw 4 lc rgb '#387DB8' title "$title2"

    set size 1,0.6
    set origin 0.0,0.00
    set bmargin 0
    set tmargin 1

    set xlabel 'Data inserted (%) - Rangemerge'
    set yrange [0:25]
    set y2range [0:200]
    plot \
   '$rng_file1' using (\$1 / $rng_xmax1 * 100):(sec2min(\$2))                             with lines lw 4 lc rgb '#E4191C' title "$title1", \
   '$rng_file2' using (\$1 / $rng_xmax2 * 100):(sec2min((\$2 / $rng_ymax2) * $rng_ymax1)) with lines lw 4 lc rgb '#387DB8' title "$title2"


    set nomultiplot


EOF
#==========================[ gnuplot embedded script ]============================


#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

inputfile="${statsfolder}/rangemerge-blocksizes-latency.totalstats"
outfile="rangemerge-blocksizes-latency"

ymax2=`cat $inputfile 2> /dev/null | awk '{if ($2 > max) max = $2;} END{print max*1.04}'`
ymax3=`cat $inputfile 2> /dev/null | awk '{if ($3 > max) max = $3;} END{print max*1.04}'`
ymax4=`cat $inputfile 2> /dev/null | awk '{if ($4 > max) max = $4;} END{print max*1.04}'`
ymax5=`cat $inputfile 2> /dev/null | awk '{if ($5 > max) max = $5;} END{print max*1.04}'`
ymax7=`cat $inputfile 2> /dev/null | awk '{if ($7 > max) max = $7;} END{print (max/60)*1.04}'`

xmin=0.8
xmax=6.2

my_print
ensure_file_exist $inputfile

style="linespoints pt 2 lw 4 lc rgb '${color}'"

tmpfile=$(mktemp)
cat ${inputfile} | awk '{print NR, $0}' > ${tmpfile}
xticklabels=`cat ${tmpfile} | awk '{if (NR>1) printf ", "; printf "\"%d\" %d", $2, $1}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    #set xtics font 'Helvetica,20'
    #set ytics font 'Helvetica,20'
    set xlabel font 'Helvetica,26'
    set ylabel font 'Helvetica,26'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_blocksize}'
    set ylabel '${ylabel_glatency}'
    set xrange [$xmin:$xmax]
    set xtics (${xticklabels})

    sec2min(x) = x/60.0
    set grid ytics

    set out '${outfolder}/${outfile}.avg.eps'
    set yrange [0:${ymax2}]
    plot '${tmpfile}' using 1:2  notitle with ${style}

    set out '${outfolder}/${outfile}.50perc.eps'
    set yrange [0:${ymax3}]
    plot '${tmpfile}' using 1:3  notitle with ${style}

    set out '${outfolder}/${outfile}.90perc.eps'
    set yrange [0:${ymax4}]
    plot '${tmpfile}' using 1:4  notitle with ${style}

    set out '${outfolder}/${outfile}.99perc.eps'
    set yrange [0:${ymax5}]
    plot '${tmpfile}' using 1:5  notitle with ${style}

    set key top right
    set ylabel '${ylabel_ins}'

    set out '${outfolder}/${outfile}.totaltime.eps'
    set yrange [0:${ymax7}]
    plot '${tmpfile}' using 1:(sec2min(\$7)) notitle with ${style}

EOF
#==========================[ gnuplot embedded script ]============================

rm $tmpfile

#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

file_cas="${statsfolder}/cassandra-l-4-getthreads.totalstats"
file_log="${statsfolder}/geometric-r-2-getthreads.totalstats"
file_geo="${statsfolder}/geometric-r-3-getthreads.totalstats"
file_rng="${statsfolder}/rangemerge-getthreads.totalstats"
file_imm="${statsfolder}/immediate-getthreads.totalstats"
file_gp2="${statsfolder}/geometric-p-2-getthreads.totalstats"

ymax2=`cat $file_gp2 $file_imm $file_rng $file_geo $file_log $file_cas 2> /dev/null | awk '{if ($2 > max) max = $2;} END{print max*1.02}'`
ymax3=`cat $file_gp2 $file_imm $file_rng $file_geo $file_log $file_cas 2> /dev/null | awk '{if ($3 > max) max = $3;} END{print max*1.02}'`
ymax4=`cat $file_gp2 $file_imm $file_rng $file_geo $file_log $file_cas 2> /dev/null | awk '{if ($4 > max) max = $4;} END{print max*1.02}'`
ymax5=`cat $file_gp2 $file_imm $file_rng $file_geo $file_log $file_cas 2> /dev/null | awk '{if ($5 > max) max = $5;} END{print max*1.02}'`
ymax7=`cat $file_gp2 $file_imm $file_rng $file_geo $file_log $file_cas 2> /dev/null | awk '{if ($7 > max) max = $7;} END{print (max/60)*1.02}'`

xmin=0
xmax=10.5

my_print
ensure_file_exist $file_cas $file_log $file_geo $file_rng $file_imm $file_gp2

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    #set xtics font 'Helvetica,20'
    #set ytics font 'Helvetica,20'
    set xlabel font 'Helvetica,26'
    set ylabel font 'Helvetica,26'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_gthreads}'
    set ylabel '${ylabel_glatency}'
    set xrange [$xmin:$xmax]

    sec2min(x) = x/60.0
    set grid ytics

    set key bottom right

    set out '${outfolder}/allmethods-getthreads.avg.eps'
    set yrange [0:$ymax2]
    plot \
    '$file_cas' using 1:2 title '$title_cas' with $style_cas, \
    '$file_log' using 1:2 title '$title_log' with $style_log, \
    '$file_geo' using 1:2 title '$title_geo' with $style_geo, \
    '$file_gp2' using 1:2 title '$title_gp2' with $style_gp2, \
    '$file_imm' using 1:2 title '$title_imm' with $style_imm, \
    '$file_rng' using 1:2 title '$title_rng' with $style_rng

    set out '${outfolder}/allmethods-getthreads.50perc.eps'
    set yrange [0:$ymax3]
    plot \
    '$file_cas' using 1:3 title '$title_cas' with $style_cas, \
    '$file_log' using 1:3 title '$title_log' with $style_log, \
    '$file_geo' using 1:3 title '$title_geo' with $style_geo, \
    '$file_gp2' using 1:3 title '$title_gp2' with $style_gp2, \
    '$file_imm' using 1:3 title '$title_imm' with $style_imm, \
    '$file_rng' using 1:3 title '$title_rng' with $style_rng

    set out '${outfolder}/allmethods-getthreads.90perc.eps'
    set yrange [0:$ymax4]
    plot \
    '$file_cas' using 1:4 title '$title_cas' with $style_cas, \
    '$file_log' using 1:4 title '$title_log' with $style_log, \
    '$file_geo' using 1:4 title '$title_geo' with $style_geo, \
    '$file_gp2' using 1:4 title '$title_gp2' with $style_gp2, \
    '$file_imm' using 1:4 title '$title_imm' with $style_imm, \
    '$file_rng' using 1:4 title '$title_rng' with $style_rng

    set out '${outfolder}/allmethods-getthreads.99perc.eps'
    set yrange [0:$ymax5]
    plot \
    '$file_cas' using 1:5 title '$title_cas' with $style_cas, \
    '$file_log' using 1:5 title '$title_log' with $style_log, \
    '$file_geo' using 1:5 title '$title_geo' with $style_geo, \
    '$file_gp2' using 1:5 title '$title_gp2' with $style_gp2, \
    '$file_imm' using 1:5 title '$title_imm' with $style_imm, \
    '$file_rng' using 1:5 title '$title_rng' with $style_rng

    set ylabel '${ylabel_ins}'

    set out '${outfolder}/allmethods-getthreads.totaltime.eps'
    set yrange [0:$ymax7]
    plot \
    '$file_imm' using 1:(sec2min(\$7)) title '$title_imm' with $style_imm, \
    '$file_gp2' using 1:(sec2min(\$7)) title '$title_gp2' with $style_gp2, \
    '$file_geo' using 1:(sec2min(\$7)) title '$title_geo' with $style_geo, \
    '$file_rng' using 1:(sec2min(\$7)) title '$title_rng' with $style_rng, \
    '$file_log' using 1:(sec2min(\$7)) title '$title_log' with $style_log, \
    '$file_cas' using 1:(sec2min(\$7)) title '$title_cas' with $style_cas

EOF
#==========================[ gnuplot embedded script ]============================


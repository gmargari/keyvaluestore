#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

file_cas="${statsfolder}/cassandra-l-4-getthrput.totalstats"
file_log="${statsfolder}/geometric-r-2-getthrput.totalstats"
file_geo="${statsfolder}/geometric-r-3-getthrput.totalstats"
file_rng="${statsfolder}/rangemerge-getthrput.totalstats"
file_imm="${statsfolder}/immediate-getthrput.totalstats"
file_gp2="${statsfolder}/geometric-p-2-getthrput.totalstats"
file_nom="${statsfolder}/nomerge-getthrput.totalstats"

#ymax2=`cat $file_imm $file_gp2 $file_rng $file_geo $file_cas $file_nom 2> /dev/null | awk '{if ($2 > max) max = $2;} END{print max*1.02}'`
#ymax3=`cat $file_imm $file_gp2 $file_rng $file_geo $file_cas $file_nom 2> /dev/null | awk '{if ($3 > max) max = $3;} END{print max*1.02}'`
#ymax4=`cat $file_imm $file_gp2 $file_rng $file_geo $file_cas $file_nom 2> /dev/null | awk '{if ($4 > max) max = $4;} END{print max*1.02}'`
#ymax5=`cat $file_imm $file_gp2 $file_rng $file_geo $file_cas $file_nom 2> /dev/null | awk '{if ($5 > max) max = $5;} END{print max*1.02}'`
#ymax7=`cat $file_imm $file_gp2 $file_rng $file_geo $file_cas $file_nom 2> /dev/null | awk '{if ($7 > max) max = $7;} END{print (max/60)*1.02}'`
ymax2=`cat $file_imm $file_gp2 $file_rng $file_geo $file_log $file_cas $file_nom 2> /dev/null | awk '{if ($2 > max) max = $2;} END{print (max/60)*1.02}'`

xmin=`cat $file_cas | awk 'BEGIN{min=1000}{if ($1 < min) min = $1;} END{print min - 1}'`
xmax=`cat $file_cas | awk '{if ($1 > max) max = $1;} END{print max + 1}'`

my_print
ensure_file_exist $file_cas $file_log $file_geo $file_rng $file_imm $file_gp2 $file_nom

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    #set xtics font 'Helvetica,20'
    #set ytics font 'Helvetica,20'
    #set xlabel font 'Helvetica,26'
    #set ylabel font 'Helvetica,26'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_gthrput}'
    set ylabel '${ylabel_glatency}'
    set xrange [$xmin:$xmax]

    sec2min(x) = x/60.0
    set grid ytics

#========================================================================================================
#    set key bottom right
#
#    set out '${outfolder}/allmethods-gthrputs.avg.eps'
#    set yrange [0:$ymax2]
#    plot \
#    '$file_nom' using 1:2 title '$title_nom' with $style_nom, \
#    '$file_cas' using 1:2 title '$title_cas' with $style_cas, \
#    '$file_log' using 1:2 title '$title_log' with $style_log, \
#    '$file_geo' using 1:2 title '$title_geo' with $style_geo, \
#    '$file_gp2' using 1:2 title '$title_gp2' with $style_gp2, \
#    '$file_rng' using 1:2 title '$title_rng' with $style_rng, \
#    '$file_imm' using 1:2 title '$title_imm' with $style_imm
#
#    set out '${outfolder}/allmethods-gthrputs.50perc.eps'
#    set yrange [0:$ymax3]
#    plot \
#    '$file_nom' using 1:3 title '$title_nom' with $style_nom, \
#    '$file_cas' using 1:3 title '$title_cas' with $style_cas, \
#    '$file_log' using 1:3 title '$title_log' with $style_log, \
#    '$file_geo' using 1:3 title '$title_geo' with $style_geo, \
#    '$file_gp2' using 1:3 title '$title_gp2' with $style_gp2, \
#    '$file_rng' using 1:3 title '$title_rng' with $style_rng, \
#    '$file_imm' using 1:3 title '$title_imm' with $style_imm
#
#    set out '${outfolder}/allmethods-gthrputs.90perc.eps'
#    set yrange [0:$ymax4]
#    plot \
#    '$file_nom' using 1:4 title '$title_nom' with $style_nom, \
#    '$file_cas' using 1:4 title '$title_cas' with $style_cas, \
#    '$file_log' using 1:4 title '$title_log' with $style_log, \
#    '$file_geo' using 1:4 title '$title_geo' with $style_geo, \
#    '$file_gp2' using 1:4 title '$title_gp2' with $style_gp2, \
#    '$file_rng' using 1:4 title '$title_rng' with $style_rng, \
#    '$file_imm' using 1:4 title '$title_imm' with $style_imm
#
#    set out '${outfolder}/allmethods-gthrputs.99perc.eps'
#    set yrange [0:$ymax5]
#    plot \
#    '$file_nom' using 1:5 title '$title_nom' with $style_nom, \
#    '$file_cas' using 1:5 title '$title_cas' with $style_cas, \
#    '$file_log' using 1:5 title '$title_log' with $style_log, \
#    '$file_geo' using 1:5 title '$title_geo' with $style_geo, \
#    '$file_gp2' using 1:5 title '$title_gp2' with $style_gp2, \
#    '$file_rng' using 1:5 title '$title_rng' with $style_rng, \
#    '$file_imm' using 1:5 title '$title_imm' with $style_imm
#
#    set key bottom right
#    set key at 40,95
#    set ylabel '${ylabel_ins}'
#
#    set out '${outfolder}/allmethods-gthrputs.totaltime.eps'
#    set yrange [0:$ymax7]
#    plot \
#    '$file_imm' using 1:(sec2min(\$7)) title '$title_imm' with $style_imm, \
#    '$file_rng' using 1:(sec2min(\$7)) title '$title_rng' with $style_rng, \
#    '$file_gp2' using 1:(sec2min(\$7)) title '$title_gp2' with $style_gp2, \
#    '$file_geo' using 1:(sec2min(\$7)) title '$title_geo' with $style_geo, \
#    '$file_log' using 1:(sec2min(\$7)) title '$title_log' with $style_log, \
#    '$file_cas' using 1:(sec2min(\$7)) title '$title_cas' with $style_cas, \
#    '$file_nom' using 1:(sec2min(\$7)) title '$title_nom' with $style_nom
#========================================================================================================

    set key bottom right
#    set key at 40,87
    set ylabel '${ylabel_ins}'

    set out '${outfolder}/allmethods-gthrputs.totaltime.eps'
    set yrange [0:$ymax2]
    plot \
    '$file_imm' using 1:(sec2min(\$2)) title '$title_imm' with $style_imm, \
    '$file_rng' using 1:(sec2min(\$2)) title '$title_rng' with $style_rng, \
    '$file_gp2' using 1:(sec2min(\$2)) title '$title_gp2' with $style_gp2, \
    '$file_geo' using 1:(sec2min(\$2)) title '$title_geo' with $style_geo, \
    '$file_log' using 1:(sec2min(\$2)) title '$title_log' with $style_log, \
    '$file_cas' using 1:(sec2min(\$2)) title '$title_cas' with $style_cas, \
    '$file_nom' using 1:(sec2min(\$2)) title '$title_nom' with $style_nom

EOF
#==========================[ gnuplot embedded script ]============================


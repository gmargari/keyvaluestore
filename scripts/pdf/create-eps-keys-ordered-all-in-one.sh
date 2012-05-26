#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

file_imm="${statsfolder}/immediate-ord-prob.totalstats"
file_rng="${statsfolder}/rangemerge-ord-prob.totalstats"
file_gp2="${statsfolder}/geometric-p-2-ord-prob.totalstats"
file_geo="${statsfolder}/geometric-r-3-ord-prob.totalstats"
file_cl2="${statsfolder}/cassandra-l-2-ord-prob.totalstats"
file_log="${statsfolder}/geometric-r-2-ord-prob.totalstats"
file_cas="${statsfolder}/cassandra-l-4-ord-prob.totalstats"
file_nom="${statsfolder}/nomerge-ord-prob.totalstats"

outfile="$outfolder/ord-prob"

my_print
ensure_file_exist $file_imm $file_rng $file_gp2 $file_geo $file_cl2 $file_log $file_cas $file_nom

xmin=-3
xmax=103

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
    set xlabel font 'Helvetica,26'
    set ylabel font 'Helvetica,26'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_keysord}'
    set xrange [${xmin}:${xmax}]

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set xtics 10    # major xticks
#    set mxtics 0    # minor xticks
    set grid ytics

    # Insertion time
    set out '${outfile}.totaltime.eps'
    set ylabel '${ylabel_ins}'
    set yrange [0:120]
    set key at ${xmax},95
    plot \
    '$file_imm' using (\$1*100):(sec2min(\$3)) title '$title_imm' with $style_imm, \
    '$file_rng' using (\$1*100):(sec2min(\$3)) title '$title_rng' with $style_rng, \
    '$file_gp2' using (\$1*100):(sec2min(\$3)) title '$title_gp2' with $style_gp2, \
    '$file_geo' using (\$1*100):(sec2min(\$3)) title '$title_geo' with $style_geo, \
    '$file_log' using (\$1*100):(sec2min(\$3)) title '$title_log' with $style_log, \
    '$file_cas' using (\$1*100):(sec2min(\$3)) title '$title_cas' with $style_cas, \
    '$file_nom' using (\$1*100):(sec2min(\$3)) title '$title_nom' with $style_nom

    # Compaction Time
    set out '${outfile}.compacttime.eps'
    set ylabel '${ylabel_comp}'
    set yrange [0:110]
    set key at 95,87
    plot \
    '$file_imm' using (\$1*100):(sec2min(\$6)) title '$title_imm' with $style_imm, \
    '$file_rng' using (\$1*100):(sec2min(\$6)) title '$title_rng' with $style_rng, \
    '$file_gp2' using (\$1*100):(sec2min(\$6)) title '$title_gp2' with $style_gp2, \
    '$file_geo' using (\$1*100):(sec2min(\$6)) title '$title_geo' with $style_geo, \
    '$file_log' using (\$1*100):(sec2min(\$6)) title '$title_log' with $style_log, \
    '$file_cas' using (\$1*100):(sec2min(\$6)) title '$title_cas' with $style_cas, \
    '$file_nom' using (\$1*100):(sec2min(\$6)) title '$title_nom' with $style_nom

    # I/O Time
    set out '${outfile}.iotime.eps'
    set ylabel '${ylabel_io}'
    set yrange [0:110]
    set key at 95,87
    plot \
    '$file_imm' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_imm' with $style_imm, \
    '$file_rng' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_rng' with $style_rng, \
    '$file_gp2' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_gp2' with $style_gp2, \
    '$file_geo' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_geo' with $style_geo, \
    '$file_log' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_log' with $style_log, \
    '$file_cas' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_cas' with $style_cas, \
    '$file_nom' using (\$1*100):(sec2min(\$10 + \$11)) title '$title_nom' with $style_nom

    # Data inserted (GB)
    set out '${outfile}.gbtransferred.eps'
    set ylabel '${ylabel_gb}'
    set yrange [0:240]
    set key at 95,195
    plot \
    '$file_imm' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_imm' with $style_imm, \
    '$file_rng' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_rng' with $style_rng, \
    '$file_gp2' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_gp2' with $style_gp2, \
    '$file_geo' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_geo' with $style_geo, \
    '$file_log' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_log' with $style_log, \
    '$file_cas' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_cas' with $style_cas, \
    '$file_nom' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title_nom' with $style_nom

EOF
#==========================[ gnuplot embedded script ]============================


#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

file_nom="${statsfolder}/nomerge-memsize.totalstats"
file_cas="${statsfolder}/cassandra-l-4-memsize.totalstats"
file_log="${statsfolder}/geometric-r-2-memsize.totalstats"
file_geo="${statsfolder}/geometric-r-3-memsize.totalstats"
file_rng="${statsfolder}/rangemerge-memsize.totalstats"
file_imm="${statsfolder}/immediate-memsize.totalstats"
file_gp2="${statsfolder}/geometric-p-2-memsize.totalstats"

my_print
ensure_file_exist $file_nom $file_cas $file_log $file_geo $file_rng $file_imm $file_gp2

file_nom_2=$(mktemp)
file_cas_2=$(mktemp)
file_log_2=$(mktemp)
file_geo_2=$(mktemp)
file_rng_2=$(mktemp)
file_imm_2=$(mktemp)
file_gp2_2=$(mktemp)

awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_nom} > ${file_nom_2}
awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_cas} > ${file_cas_2}
awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_log} > ${file_log_2}
awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_geo} > ${file_geo_2}
awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_rng} > ${file_rng_2}
awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_imm} > ${file_imm_2}
awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${file_gp2} > ${file_gp2_2}

xticklabels=`awk '{if (NR>1) printf ", "; if (NF>1) printf "\"%s\" %s", $1, ++i}' ${file_gp2}`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font 'Helvetica,20'
#    set ytics font 'Helvetica,20'
    set xlabel font 'Helvetica,26'
    set ylabel font 'Helvetica,26'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel_memsize}'
    set ylabel '${ylabel_ins}'
    set xrange [1-0.2:5+0.2]
    set yrange [0:]

    set xtics (${xticklabels}) # rotate by -45

    sec2min(x) = x/60.0
    mb2gb(x) = x/1024.0
    set grid ytics

    set key top right

    set out '${outfolder}/allmethods-memsizes.totaltime.eps'

    plot \
    '$file_imm_2' using 1:(sec2min(\$4)) title '$title_imm' with $style_imm, \
    '$file_rng_2' using 1:(sec2min(\$4)) title '$title_rng' with $style_rng, \
    '$file_gp2_2' using 1:(sec2min(\$4)) title '$title_gp2' with $style_gp2, \
    '$file_geo_2' using 1:(sec2min(\$4)) title '$title_geo' with $style_geo, \
    '$file_log_2' using 1:(sec2min(\$4)) title '$title_log' with $style_log, \
    '$file_cas_2' using 1:(sec2min(\$4)) title '$title_cas' with $style_cas, \
    '$file_nom_2' using 1:(sec2min(\$4)) title '$title_nom' with $style_nom

    set yrange [1:]
    set logscale y
    set key top right

    set out '${outfolder}/allmethods-memsizes.totaltime.log.eps'
    plot \
    '$file_imm_2' using 1:(sec2min(\$4)) title '$title_imm' with $style_imm, \
    '$file_rng_2' using 1:(sec2min(\$4)) title '$title_rng' with $style_rng, \
    '$file_gp2_2' using 1:(sec2min(\$4)) title '$title_gp2' with $style_gp2, \
    '$file_geo_2' using 1:(sec2min(\$4)) title '$title_geo' with $style_geo, \
    '$file_log_2' using 1:(sec2min(\$4)) title '$title_log' with $style_log, \
    '$file_cas_2' using 1:(sec2min(\$4)) title '$title_cas' with $style_cas, \
    '$file_nom_2' using 1:(sec2min(\$4)) title '$title_nom' with $style_nom

EOF
#==========================[ gnuplot embedded script ]============================

rm $file_nom_2 $file_cas_2 $file_log_2 $file_geo_2 $file_rng_2 $file_imm_2 $file_gp2_2

#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

#==========================================================================================
# For each one of the following:
#  - Insertion time
#  - IO time
#  - Compaction Time 
#  - GB transferred
# create a bar chart containing all methods
#==========================================================================================

file_imm="${statsfolder}/immediate.log";     every_imm="1"
file_rng="${statsfolder}/rangemerge.log";    every_rng="10"
file_gp2="${statsfolder}/geometric-p-2.log"; every_gp2="1"
file_geo="${statsfolder}/geometric-r-3.log"; every_geo="1"
file_cl2="${statsfolder}/cassandra-l-2.log"; every_cl2="1"
file_log="${statsfolder}/geometric-r-2.log"; every_log="1"
file_cas="${statsfolder}/cassandra-l-4.log"; every_cas="1"
file_nom="${statsfolder}/nomerge.log";       every_nom="1"

ensure_file_exist $file_imm $file_rng $file_gp2 $file_geo $file_cl2 $file_log $file_cas $file_nom

ymax_total=`cat $file_imm $file_rng $file_gp2 $file_geo $file_cl2 $file_log $file_cas $file_nom | awk '{if ($2 > max) max = $2;} END{print (max/60.0) * 1.2}'`
ymax_comp=`cat $file_imm $file_rng $file_gp2 $file_geo $file_cl2 $file_log $file_cas $file_nom  | awk '{if ($5 > max) max = $5;} END{print (max/60.0) * 1.2}'`
ymax_io=`cat $file_imm $file_rng $file_gp2 $file_geo $file_cl2 $file_log $file_cas $file_nom    | awk '{if ($9 + $10 > max) max = $9 + $10;} END{print (max/60.0) * 1.2}'`
ymax_gb=`cat $file_imm $file_rng $file_gp2 $file_geo $file_cl2 $file_log $file_cas $file_nom    | awk '{if ($11 + $12 > max) max = $11 + $12;} END{print (max/1024.0) * 1.2}'`

ymin_total=0.55
ymin_comp=0.25
ymin_io=0.25
ymin_gb=0.45

my_print

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    #set xtics font 'Helvetica,16'
    #set ytics font 'Helvetica,16'
    #set xlabel font 'Helvetica,16'
    #set ylabel font 'Helvetica,16'
    set terminal postscript color enhanced eps "Helvetica" 22

    #set grid ytics
    set xlabel 'Data inserted (GB)'

    set yrange [0.1:] # start y from 0+ for logaritmic scale
    set logscale y

    set key bottom right
#    set mxtics 0 # minor xticks
#    set grid xtics ytics mxtics mytics # set grid [no]xtics [no]ytics

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    #=================================================
    # Plots: Selected methods - Insertion time
    #=================================================

    set yrange [$ymin_total:$ymax_total]

    set out '${outfolder}/8methods.logyscale.totaltime.eps'
    set ylabel "$ylabel_ins"
    plot \
    '$file_imm' using (mb2gb(\$1)):(sec2min(\$2)) every $every_imm title '$title_imm' with $style_imm, \
    '$file_rng' using (mb2gb(\$1)):(sec2min(\$2)) every $every_rng title '$title_rng' with $style_rng, \
    '$file_gp2' using (mb2gb(\$1)):(sec2min(\$2)) every $every_gp2 title '$title_gp2' with $style_gp2, \
    '$file_geo' using (mb2gb(\$1)):(sec2min(\$2)) every $every_geo title '$title_geo' with $style_geo, \
    '$file_cl2' using (mb2gb(\$1)):(sec2min(\$2)) every $every_cl2 title '$title_cl2' with $style_cl2, \
    '$file_log' using (mb2gb(\$1)):(sec2min(\$2)) every $every_log title '$title_log' with $style_log, \
    '$file_cas' using (mb2gb(\$1)):(sec2min(\$2)) every $every_cas title '$title_cas' with $style_cas, \
    '$file_nom' using (mb2gb(\$1)):(sec2min(\$2)) every $every_nom title '$title_nom' with $style_nom

    #=================================================
    # Plots: Selected methods - Compaction time
    #=================================================

    set yrange [$ymin_comp:$ymax_comp]

    set out '${outfolder}/8methods.logyscale.compacttime.eps'
    set ylabel "$ylabel_comp"
    plot \
    '$file_imm' using (mb2gb(\$1)):(sec2min(\$5)) every $every_imm title '$title_imm' with $style_imm, \
    '$file_rng' using (mb2gb(\$1)):(sec2min(\$5)) every $every_rng title '$title_rng' with $style_rng, \
    '$file_gp2' using (mb2gb(\$1)):(sec2min(\$5)) every $every_gp2 title '$title_gp2' with $style_gp2, \
    '$file_geo' using (mb2gb(\$1)):(sec2min(\$5)) every $every_geo title '$title_geo' with $style_geo, \
    '$file_cl2' using (mb2gb(\$1)):(sec2min(\$5)) every $every_cl2 title '$title_cl2' with $style_cl2, \
    '$file_log' using (mb2gb(\$1)):(sec2min(\$5)) every $every_log title '$title_log' with $style_log, \
    '$file_cas' using (mb2gb(\$1)):(sec2min(\$5)) every $every_cas title '$title_cas' with $style_cas, \
    '$file_nom' using (mb2gb(\$1)):(sec2min(\$5)) every $every_nom title '$title_nom' with $style_nom

    #=================================================
    # Plots: Selected methods - IO time
    #=================================================

    set yrange [$ymin_io:$ymax_io]

    set out '${outfolder}/8methods.logyscale.iotime.eps'
    set ylabel "$ylabel_io"
    plot \
    '$file_imm' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_imm title '$title_imm' with $style_imm, \
    '$file_rng' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_rng title '$title_rng' with $style_rng, \
    '$file_gp2' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_gp2 title '$title_gp2' with $style_gp2, \
    '$file_geo' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_geo title '$title_geo' with $style_geo, \
    '$file_cl2' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_cl2 title '$title_cl2' with $style_cl2, \
    '$file_log' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_log title '$title_log' with $style_log, \
    '$file_cas' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_cas title '$title_cas' with $style_cas, \
    '$file_nom' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $every_nom title '$title_nom' with $style_nom

    #=================================================
    # Plots: Selected methods - GB transferred
    #=================================================

    set yrange [$ymin_gb:$ymax_gb]

    set out '${outfolder}/8methods.logyscale.gbtransferred.eps'
    set ylabel "$ylabel_gb"
    plot \
    '$file_imm' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_imm title '$title_imm' with $style_imm, \
    '$file_rng' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_rng title '$title_rng' with $style_rng, \
    '$file_gp2' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_gp2 title '$title_gp2' with $style_gp2, \
    '$file_geo' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_geo title '$title_geo' with $style_geo, \
    '$file_cl2' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_cl2 title '$title_cl2' with $style_cl2, \
    '$file_log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_log title '$title_log' with $style_log, \
    '$file_cas' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_cas title '$title_cas' with $style_cas, \
    '$file_nom' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $every_nom title '$title_nom' with $style_nom

EOF
#==========================[ gnuplot embedded script ]============================


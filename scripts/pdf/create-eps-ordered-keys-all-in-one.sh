#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1
outfolder="$statsfolder/eps"
mkdir -p $outfolder

#============================
# my_print()
#============================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

file1="${statsfolder}/immediate-ord-prob.totalstats";      title1="Immediate";
file2="${statsfolder}/rangemerge-ord-prob.totalstats";     title2="Rangemerge";
file3="${statsfolder}/geometric-p-2-ord-prob.totalstats";  title3="Geometric_{p=2}";
file4="${statsfolder}/geometric-r-3-ord-prob.totalstats";  title4="Geometric_{r=3}";
file5="${statsfolder}/cassandra-l-2-ord-prob.totalstats";  title5="Cassandra_{l=2}";
file6="${statsfolder}/geometric-r-2-ord-prob.totalstats";  title6="Geometric_{r=2}";
file7="${statsfolder}/cassandra-l-4-ord-prob.totalstats";  title7="Cassandra_{l=4}";
file8="${statsfolder}/nomerge-ord-prob.totalstats";        title8="Nomerge";

outfile="$outfolder/ord-prob"
xticklabels=`cat $file1 | awk '{if (NR>1) printf ", "; printf "\"%.0f\" %s", (1.0 - $1)*100, NR-1}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 22

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    #set grid ytics

    set style line 1 lw 3 pt 1 lc rgb "#000000"
    set style line 2 lw 3 pt 1 lc rgb "#888888"
    set style line 3 lw 3 pt 1 lc rgb "#994FA4"
    set style line 4 lw 3 pt 1 lc rgb "#4CB04A"
    set style line 5 lw 3 pt 1 lc rgb "#A65728"
    set style line 6 lw 3 pt 1 lc rgb "#387DB8"
    set style line 7 lw 3 pt 1 lc rgb "#FF8000"
    set style line 8 lw 3 pt 1 lc rgb "#E4191C"

    set xlabel 'Percentage of keys ordered'

    # Insertion time
    set out '$outfile.totaltime.eps'
    set ylabel 'Insertion time (min)'
    set yrange [0:120]
    set key at 95,95
    plot \
    '$file1' using (\$1*100):(sec2min(\$3)) title '$title1' with linespoints ls 1, \
    '$file2' using (\$1*100):(sec2min(\$3)) title '$title2' with linespoints ls 2, \
    '$file3' using (\$1*100):(sec2min(\$3)) title '$title3' with linespoints ls 3, \
    '$file4' using (\$1*100):(sec2min(\$3)) title '$title4' with linespoints ls 4, \
    '$file6' using (\$1*100):(sec2min(\$3)) title '$title6' with linespoints ls 6, \
    '$file8' using (\$1*100):(sec2min(\$3)) title '$title8' with linespoints ls 8

    # Data inserted (GB)
    # Insertion time
    set out '$outfile.gbtransferred.eps'
    set ylabel 'Total data transferred (GB)'
    set yrange [0:240]
    set key at 95,195
    plot \
    '$file1' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title1' with linespoints ls 1, \
    '$file2' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title2' with linespoints ls 2, \
    '$file3' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title3' with linespoints ls 3, \
    '$file4' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title4' with linespoints ls 4, \
    '$file6' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title6' with linespoints ls 6, \
    '$file8' using (\$1*100):(mb2gb(\$12 + \$13)) title '$title8' with linespoints ls 8


EOF
#==========================[ gnuplot embedded script ]============================


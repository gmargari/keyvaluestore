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

#==========================================================================================
# For each one of the following:
#  - Insertion time
#  - IO time
#  - Compaction Time 
#  - GB transferred
# create a bar chart containing all methods
#==========================================================================================

file1="${statsfolder}/immediate.log";      title1="Immediate";        printevery1="1"
file2="${statsfolder}/rangemerge.log";     title2="Rangemerge";       printevery2="10"
file3="${statsfolder}/geometric-p-2.log";  title3="Geometric_{p=2}";  printevery3="1"
file4="${statsfolder}/geometric-r-3.log";  title4="Geometric_{r=3}";  printevery4="1"
file5="${statsfolder}/cassandra-l-2.log";  title5="Cassandra_{l=2}";  printevery5="1"
file6="${statsfolder}/geometric-r-2.log";  title6="Geometric_{r=2}";  printevery6="1"
file7="${statsfolder}/cassandra-l-4.log";  title7="Cassandra_{l=4}";  printevery7="1"
file8="${statsfolder}/nomerge.log";        title8="Nomerge";          printevery8="1"

ymin_total=0.55
ymin_comp=0.25
ymin_io=0.25
ymin_gb=0.45

ymax_total=`cat $file1 $file2 $file3 $file4 $file5 $file6 $file7 $file8 | awk '{if ($2 > max) max = $2;} END{print (max/60.0) * 1.2}'`
ymax_comp=`cat $file1 $file2 $file3 $file4 $file5 $file6 $file7 $file8  | awk '{if ($5 > max) max = $5;} END{print (max/60.0) * 1.2}'`
ymax_io=`cat $file1 $file2 $file3 $file4 $file5 $file6 $file7 $file8    | awk '{if ($9 + $10 > max) max = $9 + $10;} END{print (max/60.0) * 1.2}'`
ymax_gb=`cat $file1 $file2 $file3 $file4 $file5 $file6 $file7 $file8    | awk '{if ($11 + $12 > max) max = $11 + $12;} END{print (max/1024.0) * 1.2}'`

ylabel_ins='Insertion time (min)'
ylabel_comp='Compaction time (min)'
ylabel_io='I/O time (min)'
ylabel_gb='Total data transferred (GB)'

my_print "all methods"

gnuplot << EOF

    #set xtics font 'Helvetica,16'
    #set ytics font 'Helvetica,16'
    #set xlabel font 'Helvetica,16'
    #set ylabel font 'Helvetica,16'
    set terminal postscript color enhanced eps "Helvetica" 20

    #set grid ytics
    set xlabel 'Data inserted (GB)'

    set yrange [0.1:] # start y from 0+ for logaritmic scale
    set logscale y

    set key bottom right
#    set mxtics 0 # minor xticks
#    set grid xtics ytics mxtics mytics # set grid [no]xtics [no]ytics

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set style line 1 lw 3 pt 1 lc rgb "#000000"
    set style line 2 lw 3 pt 1 lc rgb "#888888"
    set style line 3 lw 3 pt 1 lc rgb "#994FA4"
    set style line 4 lw 3 pt 1 lc rgb "#4CB04A"
    set style line 5 lw 3 pt 1 lc rgb "#A65728"
    set style line 6 lw 3 pt 1 lc rgb "#387DB8"
    set style line 7 lw 3 pt 1 lc rgb "#FF8000"
    set style line 8 lw 3 pt 1 lc rgb "#E4191C"


    #=================================================
    # Plots: Selected methods - Insertion time
    #=================================================

    set yrange [$ymin_total:$ymax_total]

    set out '${outfolder}/8methods.logyscale.totaltime.eps'
    set ylabel "$ylabel_ins"
    plot \
    '$file1' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery1 title '$title1' with linespoints ls 1, \
    '$file2' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery2 title '$title2' with linespoints ls 2, \
    '$file3' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery3 title '$title3' with linespoints ls 3, \
    '$file4' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery4 title '$title4' with linespoints ls 4, \
    '$file5' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery5 title '$title5' with linespoints ls 5, \
    '$file6' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery6 title '$title6' with linespoints ls 6, \
    '$file7' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery7 title '$title7' with linespoints ls 7, \
    '$file8' using (mb2gb(\$1)):(sec2min(\$2)) every $printevery8 title '$title8' with linespoints ls 8

    #=================================================
    # Plots: Selected methods - Compaction time
    #=================================================

    set yrange [$ymin_comp:$ymax_comp]

    set out '${outfolder}/8methods.logyscale.compacttime.eps'
    set ylabel "$ylabel_comp"
    plot \
    '$file1' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery1 title '$title1' with linespoints ls 1, \
    '$file2' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery2 title '$title2' with linespoints ls 2, \
    '$file3' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery3 title '$title3' with linespoints ls 3, \
    '$file4' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery4 title '$title4' with linespoints ls 4, \
    '$file5' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery5 title '$title5' with linespoints ls 5, \
    '$file6' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery6 title '$title6' with linespoints ls 6, \
    '$file7' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery7 title '$title7' with linespoints ls 7, \
    '$file8' using (mb2gb(\$1)):(sec2min(\$5)) every $printevery8 title '$title8' with linespoints ls 8

    #=================================================
    # Plots: Selected methods - IO time
    #=================================================

    set yrange [$ymin_io:$ymax_io]

    set out '${outfolder}/8methods.logyscale.iotime.eps'
    set ylabel "$ylabel_io"
    plot \
    '$file1' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery1 title '$title1' with linespoints ls 1, \
    '$file2' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery2 title '$title2' with linespoints ls 2, \
    '$file3' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery3 title '$title3' with linespoints ls 3, \
    '$file4' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery4 title '$title4' with linespoints ls 4, \
    '$file5' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery5 title '$title5' with linespoints ls 5, \
    '$file6' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery6 title '$title6' with linespoints ls 6, \
    '$file7' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery7 title '$title7' with linespoints ls 7, \
    '$file8' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) every $printevery8 title '$title8' with linespoints ls 8

    #=================================================
    # Plots: Selected methods - GB transferred
    #=================================================

    set yrange [$ymin_gb:$ymax_gb]

    set out '${outfolder}/8methods.logyscale.gbtransferred.eps'
    set ylabel "$ylabel_gb"
    plot \
    '$file1' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery1 title '$title1' with linespoints ls 1, \
    '$file2' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery2 title '$title2' with linespoints ls 2, \
    '$file3' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery3 title '$title3' with linespoints ls 3, \
    '$file4' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery4 title '$title4' with linespoints ls 4, \
    '$file5' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery5 title '$title5' with linespoints ls 5, \
    '$file6' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery6 title '$title6' with linespoints ls 6, \
    '$file7' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery7 title '$title7' with linespoints ls 7, \
    '$file8' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every $printevery8 title '$title8' with linespoints ls 8

EOF


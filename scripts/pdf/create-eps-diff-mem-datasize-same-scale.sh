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

#--------------------------------------------------------------------------------
# You may need to "truncate" contents of files in order for the 2 plots
# to overlap better (e.g. if M1 = 128 MB, D1 = 2500 MB and M2 = 512 MB, keep
# from second file up to 10000 MB of data).
#--------------------------------------------------------------------------------

for i in `seq 1 2`; do

    if [ $i == "1" ]; then
        file1=${statsfolder}/rangemerge-memsize-0256-datasize-5000.log
        title1="M = 256MB, Data = 5GB"
        file2=${statsfolder}/rangemerge-memsize-2048-datasize-40000.log
        title2="M = 2048MB, Data = 40GB"
        outputfile=${outfolder}/rangemerge-diff-mem-datasize-same-scale.eps
        legendx=65; legendy=22.5;

        my_print "rangemerge"
    else 
        file1=${statsfolder}/geometric-r-2-memsize-0256-datasize-5000.log
        title1="M = 256MB, Data = 5GB"
        file2=${statsfolder}/geometric-r-2-memsize-2048-datasize-40000.log
        title2="M = 2048MB, Data = 40GB"
        outputfile=${outfolder}/geometric-r-2-diff-mem-datasize-same-scale.eps
        legendx=65; legendy=15;

        my_print "Geometric r = 2"
    fi


    if [ ! -f $file1 -o ! -f $file2 ]; then
        echo "Error: $file1 and/or $file2 does not exist" >&2;
        exit 1;
    fi

    xmax1=`cat $file1 | awk '{if ($1 > max) max=$1;} END{print max}'`
    xmax2=`cat $file2 | awk '{if ($1 > max) max=$1;} END{print max}'`

    ymax1=`cat $file1 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`
    ymax2=`cat $file2 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`

    xmax1=`echo "$xmax1 * 1.01" | bc -l`
    xmax2=`echo "$xmax2 * 1.01" | bc -l`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font   'Helvetica,21'
#    set ytics font   'Helvetica,21'
#    set y2tics font  'Helvetica,21'
#    set xlabel font  'Helvetica,22'
#    set ylabel font  'Helvetica,22'
#    set y2label font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 19
    set xlabel 'Data inserted (%)'

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    # legend position
#    set key top left
    set key at $legendx, $legendy

    set yrange [0:${ymax1}+1]
    set ylabel 'Insertion time (min)'
    set ytic nomirror
    set y2range [0:${ymax2}+1]
    set y2label 'Insertion time (min)'
    set y2tic nomirror

    set out '${outputfile}'

    plot \
   '$file1' using (\$1 / $xmax1 * 100):(sec2min(\$2))                     with lines lw 4 lc rgb '#E4191C' title "$title1", \
   '$file2' using (\$1 / $xmax2 * 100):(sec2min((\$2 / $ymax2) * $ymax1)) with lines lw 4 lc rgb '#387DB8' title "$title2"

EOF
#==========================[ gnuplot embedded script ]============================

done

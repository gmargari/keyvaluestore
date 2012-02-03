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

inputfile1=${statsfolder}/geometric-r-2-memsize-0128-datasize-2500.log
title1="Logarithmic, M = 128MB, Data = 2.5GB"
inputfile2=${statsfolder}/geometric-r-2-memsize-2048-datasize-40000.log
title2="Logarithmic, M = 2GB, Data = 40GB"

outputfile=${outfolder}/diff-mem-datasize-same-scale.eps

if [ ! -f $inputfile1 -o ! -f $inputfile2 ]; then
    echo "Error: $inputfile1 and/or $inputfile2 does not exist" >&2;
    exit 1;
fi

file1=$inputfile1
file2=$inputfile2

# create a new file, adding for each <x,y> pair a new <x,prev_y> pair, for better visualization in plots:
#file1="$(mktemp)"
#file2="$(mktemp)"
#cat ${inputfile1} | awk '{if (prev) print $1 prev; print $0; prev=""; for(i=2;i<=NF;i++) prev=prev" "$(i)}' > ${file1}
#cat ${inputfile2} | awk '{if (prev) print $1 prev; print $0; prev=""; for(i=2;i<=NF;i++) prev=prev" "$(i)}' > ${file2}

xmax1=`cat $file1 | awk '{if ($1 > max) max=$1;} END{print max}'`
xmax2=`cat $file2 | awk '{if ($1 > max) max=$1;} END{print max}'`

ymax1=`cat $file1 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`
ymax2=`cat $file2 | awk '{if ($2 > max) max=$2;} END{print max/60.0}'`

xmax1=`echo "$xmax1 * 1.01" | bc -l`
xmax2=`echo "$xmax2 * 1.01" | bc -l`

my_print "logarithmic"

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
#    set key top right
    set key at 0.88,7.5

#    set size ratio 0.4

    set yrange [0:${ymax1}+1]
    set ylabel 'Insertion time (min)'
    set ytic nomirror
    set y2range [0:${ymax2}+1]
    set y2label 'Insertion time (min)'
    set y2tic nomirror

    set out '${outputfile}'

    plot \
   '$file1' using (\$1 / $xmax1):(sec2min(\$2))                     with linespoints lw 4 lc rgb '#E4191C' title "$title1", \
   '$file2' using (\$1 / $xmax2):(sec2min((\$2 / $ymax2) * $ymax1)) with linespoints lw 4 lc rgb '#387DB8' title "$title2"

EOF
#==========================[ gnuplot embedded script ]============================

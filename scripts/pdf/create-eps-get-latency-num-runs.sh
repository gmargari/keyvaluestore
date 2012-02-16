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
# Logarithmic: Number of runs vs range get time
#==========================================================================================

file="logarithmic.searchlog"
inputfile="${statsfolder}/${file}"
outputfile="${outfolder}/${file}"

# check that file exists
if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

my_print $file

tmpfile1="$(mktemp)"
tmpfile2="$(mktemp)"
tmpfile3="$(mktemp)"
tmpfile4="$(mktemp)"

cat $inputfile | awk '{if ($0 ~ "^ ") print $1, $15}' > $tmpfile1
cat $inputfile | awk '{if ($0 ~ "^ ") mbins=$1; if ($1 == "avg_rng_get_ms") print mbins, $2}' > $tmpfile2

# create a new file, adding for each <x,y> pair a new <x,prev_y> pair, for better visualization in plots:
cat ${tmpfile1} | awk '{if (prev) print $1, prev; print $0; prev=$2}' > ${tmpfile3}
cat ${tmpfile2} | awk '{if (prev) print $1, prev; print $0; prev=$2}' > ${tmpfile4}

ymax=`cat $tmpfile3 | awk '{if ($2 > max) max=$2;} END{print max}'`
y2max=`cat $tmpfile4 | awk '{if ($2 > max) max=$2;} END{print max}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set y2tics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
#    set y2label font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 20
    set xlabel 'Data inserted (GB)'
#    set x2label 'Read cost' font 'Helvetica Bold'

    # legend position
    # set key top left
    set key at 5.5,4.5

    set yrange [0:${ymax}+1]
    set ylabel 'Number of disk files'
    set ytic nomirror
    set y2range [0:${y2max}+1]
    set y2label 'Range get latency (ms)'
    set y2tic nomirror

    mb2gb(x) = x/1024.0

    set out '${outputfile}.eps'

    plot \
    '${tmpfile1}' using (mb2gb(\$1)):2                        with linespoints lw 4 lc rgb '#E4191C' title 'Number of disk files' , \
    '${tmpfile2}' using (mb2gb(\$1)):((\$2 / $y2max) * $ymax) with linespoints lw 4 lc rgb '#387DB8' title 'Range get latency' 

EOF
#==========================[ gnuplot embedded script ]============================

rm $tmpfile1 $tmpfile2 $tmpfile3 $tmpfile4
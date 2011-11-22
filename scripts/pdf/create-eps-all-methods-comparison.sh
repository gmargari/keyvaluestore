#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1
outfolder="$statsfolder/eps"
inputfile_with_linenumbers="/tmp/gnuplottmp" # tmp file
mkdir -p $outfolder

#============================
# my_print()
#============================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

my_print "all methods"

gnuplot << EOF

#set xtics font 'Helvetica,16'
#set ytics font 'Helvetica,16'
#set xlabel font 'Helvetica,16'
#set ylabel font 'Helvetica,16'
set terminal postscript color enhanced eps "Helvetica" 20
mb2gb(x) = x/1024.0
add(a,b) = a+b

set style line 1 lt 1 lw 2 lc rgb "red"
set style line 2 lt 1 lw 2 lc rgb "#228B22"
set style line 3 lt 1 lw 2 lc rgb "blue"
set style line 4 lt 1 lw 2 lc rgb "#FF1493"
set style line 5 lt 1 lw 2 lc rgb "cyan"
set style line 6 lt 1 lw 2 lc rgb "green"
set style line 7 lt 1 lw 2 lc rgb "black"
set style line 8 lt 1 lw 2 lc rgb "orange"
set style line 9 lt 1 lw 4 lc rgb "yellow"
set style line 10 lt 1 lw 2 lc rgb "#777777"

#=================================================
# IO size plots with lines for all methods
#=================================================


set yrange [0:] # start y from 0
set xlabel 'Data inserted (GB)'
set key left top

#--------------------------------------------
# Plots: All methods - Insertion time
#--------------------------------------------
set out '${outfolder}/allmethods.totaltime.eps'
set ylabel 'Insertion time (s)'

plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):2 title 'Immediate'       with lines ls 1, \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):2 title 'Rangemerge'      with lines ls 2, \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):2 title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):2 title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):2 title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):2 title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):2 title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):2 title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):2 title 'Nomerge'         with lines ls 10

set out '${outfolder}/allmethods-no-imm-no-rngmrg.totaltime.eps'
set ylabel 'Insertion time (s)'

plot \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):2 title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):2 title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):2 title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):2 title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):2 title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):2 title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):2 title 'Nomerge'         with lines ls 10

#--------------------------------------------
# Plots: All methods - Compaction time
#--------------------------------------------
set out '${outfolder}/allmethods.compacttime.eps'
set ylabel 'Compaction time (s)'

plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):5 title 'Immediate'       with lines ls 1, \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):5 title 'Rangemerge'      with lines ls 2, \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):5 title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):5 title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):5 title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):5 title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):5 title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):5 title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):5 title 'Nomerge'         with lines ls 10

set out '${outfolder}/allmethods-no-imm-no-rngmrg.compacttime.eps'
set ylabel 'Compaction time (s)'

plot \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):5 title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):5 title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):5 title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):5 title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):5 title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):5 title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):5 title 'Nomerge'         with lines ls 10

#--------------------------------------------
# Plots: All methods - I/O time
#--------------------------------------------
set out '${outfolder}/allmethods.iotime.eps'
set ylabel 'I/O time (s)'

plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):(\$9 + \$10) title 'Immediate'       with lines ls 1, \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):(\$9 + \$10) title 'Rangemerge'      with lines ls 2, \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):(\$9 + \$10) title 'Nomerge'         with lines ls 10


set out '${outfolder}/allmethods-no-imm-no-rngmrg.iotime.eps'
set ylabel 'I/O time (s)'

plot \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):(\$9 + \$10) title 'Nomerge'         with lines ls 10

#--------------------------------------------
# Plots: All methods - Total data transferred (GB)
#--------------------------------------------
set out '${outfolder}/allmethods.gbtransferred.eps'
set ylabel 'Total data transferred (GB)'

plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Immediate'       with lines ls 1, \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Rangemerge'      with lines ls 2, \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Nomerge'         with lines ls 10

set out '${outfolder}/allmethods-no-imm-no-rngmrg.gbtransferred.eps'
set ylabel 'Total data transferred (GB)'

plot \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=2}' with lines ls 3, \
'${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=3}' with lines ls 4, \
'${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=4}' with lines ls 7, \
'${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=4}' with lines ls 5, \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=3}' with lines ls 8, \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=2}' with lines ls 6, \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Nomerge'         with lines ls 10



#-------
# test
#-------
# set style line 1 lt 1 lw 2 lc rgb "red"
# set style line 2 lt 2 lw 2 lc rgb "#228B22"
# set style line 3 lt 3 lw 2 lc rgb "blue"
# set style line 4 lt 6 lw 2 lc rgb "#FF1493"
# set style line 5 lt 1 lw 2 lc rgb "cyan"
# set style line 6 lt 2 lw 2 lc rgb "green"
# set style line 7 lt 3 lw 2 lc rgb "black"
# set style line 8 lt 6 lw 2 lc rgb "orange"
# set style line 9 lt 1 lw 4 lc rgb "yellow"
#
# set out '${outfolder}/allmethods.fragm.eps'
# set ylabel 'Number of runs'
#
# plot \
# '${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):15 title 'Geometric_{r=2}' with lines ls 6, \
# '${statsfolder}/geometric-p-4.log' using (mb2gb(\$1)):15 title 'Geometric_{p=4}' with lines ls 5, \
# '${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):15 title 'Geometric_{r=3}' with lines ls 8, \
# '${statsfolder}/geometric-r-4.log' using (mb2gb(\$1)):15 title 'Geometric_{r=4}' with lines ls 7, \
# '${statsfolder}/geometric-p-3.log' using (mb2gb(\$1)):15 title 'Geometric_{p=3}' with lines ls 4, \
# '${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):15 title 'Geometric_{p=2}' with lines ls 3, \
# '${statsfolder}/immediate.log'     using (mb2gb(\$1)):15 title 'Immediate'       with lines ls 1, \
# '${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):15 title 'Rangemerge'      with lines ls 2
#
#
#
# # Logarithmic with memory sizes 128MB, 256MB, 512MB, 1024MB
#
# set out '${outfolder}/geometric-r-2-memsizes.totaltime.eps'
# set ylabel 'Insertion time (s)'
# plot \
# '${statsfolder}/geometric-p-2-memsize-0128.log' using (mb2gb(\$1)):2 title 'M =  128MB' with lines ls 1, \
# '${statsfolder}/geometric-p-2-memsize-0256.log' using (mb2gb(\$1)):2 title 'M =  256MB' with lines ls 2, \
# '${statsfolder}/geometric-p-2-memsize-0512.log' using (mb2gb(\$1)):2 title 'M =  512MB' with lines ls 3, \
# '${statsfolder}/geometric-p-2-memsize-1024.log' using (mb2gb(\$1)):2 title 'M = 1024MB' with lines ls 4
#
# set out '${outfolder}/geometric-r-2-memsizes.gbtransferred.eps'
# set ylabel 'Total data transferred (GB)'
# plot \
# '${statsfolder}/geometric-p-2-memsize-0128.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'M =  128MB' with lines ls 1, \
# '${statsfolder}/geometric-p-2-memsize-0256.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'M =  256MB' with lines ls 2, \
# '${statsfolder}/geometric-p-2-memsize-0512.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'M =  512MB' with lines ls 3, \
# '${statsfolder}/geometric-p-2-memsize-1024.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'M = 1024MB' with lines ls 4
#
# set out '${outfolder}/geometric-r-2-memsizes.iotime.eps'
# set ylabel 'I/O time (s)'
# plot \
# '${statsfolder}/geometric-p-2-memsize-0128.log' using (mb2gb(\$1)):(\$9 + \$10) title 'M =  128MB' with lines ls 1, \
# '${statsfolder}/geometric-p-2-memsize-0256.log' using (mb2gb(\$1)):(\$9 + \$10) title 'M =  256MB' with lines ls 2, \
# '${statsfolder}/geometric-p-2-memsize-0512.log' using (mb2gb(\$1)):(\$9 + \$10) title 'M =  512MB' with lines ls 3, \
# '${statsfolder}/geometric-p-2-memsize-1024.log' using (mb2gb(\$1)):(\$9 + \$10) title 'M = 1024MB' with lines ls 4



#--------------------------------------------
# Plots: 6 methods - Compaction time
#--------------------------------------------
set yrange [1:] # start y from 0
set logscale y
unset style line

set out '${outfolder}/6methods.logyscale.totaltime.eps'
set ylabel 'Insertion time (s)'
set x2label 'Update cost' font 'Helvetica Bold'
plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):2 title 'Immediate'       with lines lt -1 lw 2 lc rgb "black", \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):2 title 'Rangemerge'      with lines lt  1 lw 2 lc rgb "black", \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):2 title 'Geometric_{p=2}' with lines lt  2 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):2 title 'Geometric_{r=3}' with lines lt  3 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):2 title 'Geometric_{r=2}' with lines lt  4 lw 2 lc rgb "black", \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):2 title 'Nomerge'         with lines lt  5 lw 2 lc rgb "black"
set x2label ''

set out '${outfolder}/6methods.logyscale.compacttime.eps'
set ylabel 'Compaction time (s)'
plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):5 title 'Immediate'       with lines lt -1 lw 2 lc rgb "black", \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):5 title 'Rangemerge'      with lines lt  1 lw 2 lc rgb "black", \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):5 title 'Geometric_{p=2}' with lines lt  2 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):5 title 'Geometric_{r=3}' with lines lt  3 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):5 title 'Geometric_{r=2}' with lines lt  4 lw 2 lc rgb "black", \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):5 title 'Nomerge'         with lines lt  5 lw 2 lc rgb "black"

set out '${outfolder}/6methods.logyscale.iotime.eps'
set ylabel 'I/O time (s)'
plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):(\$9 + \$10) title 'Immediate'       with lines lt -1 lw 2 lc rgb "black", \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):(\$9 + \$10) title 'Rangemerge'      with lines lt  1 lw 2 lc rgb "black", \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{p=2}' with lines lt  2 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=3}' with lines lt  3 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):(\$9 + \$10) title 'Geometric_{r=2}' with lines lt  4 lw 2 lc rgb "black", \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):(\$9 + \$10) title 'Nomerge'         with lines lt  5 lw 2 lc rgb "black"

set out '${outfolder}/6methods.logyscale.gbtransferred.eps'
set ylabel 'Total data transferred (GB)'
plot \
'${statsfolder}/immediate.log'     using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Immediate'       with lines lt -1 lw 2 lc rgb "black", \
'${statsfolder}/rangemerge.log'    using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Rangemerge'      with lines lt  1 lw 2 lc rgb "black", \
'${statsfolder}/geometric-p-2.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{p=2}' with lines lt  2 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-3.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=3}' with lines lt  3 lw 2 lc rgb "black", \
'${statsfolder}/geometric-r-2.log' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Geometric_{r=2}' with lines lt  4 lw 2 lc rgb "black", \
'${statsfolder}/nomerge.log'       using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) title 'Nomerge'         with lines lt  5 lw 2 lc rgb "black"

EOF



#==========================================================================================
# For each memory size, plot all methods in a bar chart
#==========================================================================================

memsizes=( '0128' '0256' '0512' '1024' '2048' )

for i in `seq 0 $(( ${#memsizes[*]} - 1 ))`; do

    inputfile="${statsfolder}/allmethods-memsize-${memsizes[$i]}.totalstats"
    outputfile="${outfolder}/allmethods-memsize-${memsizes[$i]}"

    # check if file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; exit 1; fi

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", NR, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%s\" %s", $1, NR}'`

    my_print "memsize ${memsizes[$i]}"

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    set xtics (${xticklabels}) rotate by -45
    set yrange [0:] # start y from 0
    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,18'
#    set ytics font 'Helvetica,18'
#    set xlabel font 'Helvetica,21'
#    set ylabel font 'Helvetica,21'
    set terminal postscript color enhanced eps "Helvetica" 26
    set xlabel ''

    set size 0.8,1.0

    set xtics nomirror

    mb2gb(x) = x/1024.0

    set boxwidth 0.50

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set ylabel 'Insertion time (s)'
    plot '${inputfile_with_linenumbers}' using 1:4 notitle with boxes fill pattern 3 lc rgb "black"

    # Compaction Time
    set out '${outputfile}.compacttime.eps'
    set ylabel 'Compaction time (s)'
    plot '${inputfile_with_linenumbers}' using 1:7 notitle with boxes fill pattern 3 lc rgb "black"

    # I/O Time
    set out '${outputfile}.iotime.eps'
    set ylabel 'I/O time (s)'
    plot '${inputfile_with_linenumbers}' using 1:(\$11 + \$12) notitle with boxes fill pattern 3 lc rgb "black"

    # Total data transferred (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel 'Total data transferred (GB)'
    plot '${inputfile_with_linenumbers}' using 1:(mb2gb(\$13 + \$14)) notitle with boxes fill pattern 3 lc rgb "black"



EOF
#==========================[ gnuplot embedded script ]============================

done



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
# For each memory size, plot all methods in a bar chart
#==========================================================================================

inputfile_with_linenumbers="/tmp/gnuplottmp" # tmp file

color="#387DB8"
color2="#BBBBBB"

memsizes=( '0128' '0256' '0512' '1024' '2048' )

ylabel_ins='Insertion time (min)'
ylabel_comp='Compaction time (min)'
ylabel_io='I/O time (min)'
ylabel_gb='Total data transferred (GB)'

for i in `seq 0 $(( ${#memsizes[*]} - 1 ))`; do

    inputfile="${statsfolder}/allmethods-memsize-${memsizes[$i]}.totalstats"
    outputfile="${outfolder}/allmethods-memsize-${memsizes[$i]}"

    # check if file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%s\" %s", $1, NR-1}'`

    my_print "memsize ${memsizes[$i]}"

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    set xtics (${xticklabels}) rotate by -45
    set yrange [0:] # start y from 0
#    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,18'
#    set ytics font 'Helvetica,18'
#    set xlabel font 'Helvetica,21'
#    set ylabel font 'Helvetica,21'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel ''

    #set grid ytics

    set size 0.8,1.0

    set xtics nomirror

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms
    set key invert

    set style line 1 lt 1 lw 2 lc rgb "$color"
    set style line 2 lt 1 lw 2 lc rgb "$color2"

    # Insertion time - Break down in IO / Mem
    set out '${outputfile}.totaltime.breakdown.eps'
    set ylabel "$ylabel_ins"
    plot \
    '${inputfile_with_linenumbers}' using (sec2min(\$11 + \$12))         ls 1 title 'Disk I/O', \
    '${inputfile_with_linenumbers}' using (sec2min(\$4 - (\$11 + \$12))) ls 2 title 'Mem \& CPU' 

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set ylabel "$ylabel_ins"
    plot '${inputfile_with_linenumbers}' using (sec2min(\$4)) ls 1 notitle

    # Compaction Time
    set out '${outputfile}.compacttime.eps'
    set ylabel "$ylabel_comp"
    plot '${inputfile_with_linenumbers}' using (sec2min(\$7)) ls 1 notitle

    # I/O Time
    set out '${outputfile}.iotime.eps'
    set ylabel "$ylabel_io"
    plot '${inputfile_with_linenumbers}' using (sec2min(\$11 + \$12)) ls 1 notitle

    # Total data transferred (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel "$ylabel_gb"
    plot '${inputfile_with_linenumbers}' using (mb2gb(\$13 + \$14)) ls 1 notitle
EOF
#==========================[ gnuplot embedded script ]============================

done



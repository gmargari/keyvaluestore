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
# For each method, and a specific parameter with different values, create a bar chart for:
#  - Insertion time
#  - IO time
#  - Compaction Time 
#  - GB transferred
#==========================================================================================

inputfile_with_linenumbers="/tmp/gnuplottmp" # tmp file
newinputfile="/tmp/gnuplottmp2" # tmp file

color="#387DB8"
#ymax=50

#files=( 'rangemerge-blocksize-0064-ord-prob.totalstats' 'rangemerge-blocksize-0128-ord-prob.totalstats' \
#        'rangemerge-blocksize-0256-ord-prob.totalstats' 'logarithmic-ord-prob.totalstats' )
files=( 'geometric-p-2-ord-prob.totalstats' 'geometric-p-3-ord-prob.totalstats' 'geometric-p-4-ord-prob.totalstats' \
        'geometric-r-2-ord-prob.totalstats' 'geometric-r-3-ord-prob.totalstats' 'geometric-r-4-ord-prob.totalstats' \
        'cassandra-l-2-ord-prob.totalstats' 'cassandra-l-3-ord-prob.totalstats' 'cassandra-l-4-ord-prob.totalstats' \
        'rangemerge-ord-prob.totalstats' 'nomerge-ord-prob.totalstats' 'immediate-ord-prob.totalstats' )
xlabel='Percentage of keys ordered (%)'

# for each file in array 'files' create a number of eps
for i in `seq 0 $(( ${#files[*]} - 1 ))`; do

    file=${files[$i]}
    inputfile="${statsfolder}/$file"
    outputfile="${outfolder}/$file"

    fontsize=32

    # check that file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%.0f\" %s", (1.0 - $1)*100, NR-1}'`

    ylabel_ins='Insertion time (min)'
    ylabel_comp='Compaction time (min)'
    ylabel_io='I/O time (min)'
    ylabel_gb='Total data transferred (GB)'

    my_print $file

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    # mb_ins(1) Ttotal(2) Tcompac(3) Tput(4) Tmerge(5) Tfree(6) Tcmrest(7) Tmem(8) Tread(9) Twrite(10) mb_read(11) mb_writ(12) reads(13) writes(14) runs(15)

    set xtics (${xticklabels})
    set yrange [0:${ymax}] # start y from 0
#    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,22'
#    set ytics font 'Helvetica,22'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" $fontsize
    set xlabel '${xlabel}'

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set grid ytics

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms
    set key invert

    set style line 1 lt 1 lw 2 lc rgb "$color"

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

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel "$ylabel_gb"
    plot '${inputfile_with_linenumbers}' using (mb2gb(\$13 + \$14)) ls 1 notitle

EOF
#==========================[ gnuplot embedded script ]============================

done


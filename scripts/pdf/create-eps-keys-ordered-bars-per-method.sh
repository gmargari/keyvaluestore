#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

files=( 'geometric-p-2-ord-prob.totalstats' 'geometric-p-3-ord-prob.totalstats' 'geometric-p-4-ord-prob.totalstats' \
        'geometric-r-2-ord-prob.totalstats' 'geometric-r-3-ord-prob.totalstats' 'geometric-r-4-ord-prob.totalstats' \
        'cassandra-l-2-ord-prob.totalstats' 'cassandra-l-3-ord-prob.totalstats' 'cassandra-l-4-ord-prob.totalstats' \
        'rangemerge-ord-prob.totalstats' 'nomerge-ord-prob.totalstats' 'immediate-ord-prob.totalstats' )

# for each file in array 'files' create a number of eps
inputfile_with_linenumbers="$(mktemp)"
for file in ${files[@]}; do

    inputfile="${statsfolder}/$file"
    outputfile="${outfolder}/$file"

    my_print $file
    ensure_file_exist ${inputfile}

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%.0f\" %s", $1*100, NR-1}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,22'
#    set ytics font 'Helvetica,22'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 32
    set xlabel '${xlabel_keysord}'
    set xtics (${xticklabels})
    set yrange [0:${ymax}]

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set grid ytics

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms
    set key invert

    set style line 1 lt 1 lw 2 lc rgb '${color}'

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set ylabel '${ylabel_ins}'
    plot '${inputfile_with_linenumbers}' using (sec2min(\$4)) ls 1 notitle

    # Compaction Time
    set out '${outputfile}.compacttime.eps'
    set ylabel '${ylabel_comp}'
    plot '${inputfile_with_linenumbers}' using (sec2min(\$7)) ls 1 notitle

    # I/O Time
    set out '${outputfile}.iotime.eps'
    set ylabel '${ylabel_io}'
    plot '${inputfile_with_linenumbers}' using (sec2min(\$11 + \$12)) ls 1 notitle

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel '${ylabel_gb}'
    plot '${inputfile_with_linenumbers}' using (mb2gb(\$13 + \$14)) ls 1 notitle

EOF
#==========================[ gnuplot embedded script ]============================

done

rm $inputfile_with_linenumbers

#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

files=(  'rangemerge-memsize.totalstats' 'immediate-memsize.totalstats' 'nomerge-memsize.totalstats' \
         'geometric-p-4-memsize.totalstats' 'geometric-p-3-memsize.totalstats' 'geometric-p-2-memsize.totalstats' \
         'geometric-r-4-memsize.totalstats' 'geometric-r-3-memsize.totalstats' 'geometric-r-2-memsize.totalstats' \
         'cassandra-l-4-memsize.totalstats' 'cassandra-l-3-memsize.totalstats' 'cassandra-l-2-memsize.totalstats' )

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
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%s\" %s", $1, NR-1}'`

    if [ $file == "rangemerge-blocksize.totalstats" -o $file == "rangemerge-flushmem.totalstats" ]; then
        fontsize=22
    else
        fontsize=32
    fi

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,22'
#    set ytics font 'Helvetica,22'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" $fontsize
    set xlabel '${xlabel_memsize}'
    set xtics (${xticklabels})
    set yrange [0:] # start y from 0

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms
    set key invert

    set grid ytics

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

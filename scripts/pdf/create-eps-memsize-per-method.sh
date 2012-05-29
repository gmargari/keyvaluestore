#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

methods=( 'immediate' 'rangemerge' 'geometric-p-2' 'geometric-r-3' 'geometric-r-2' \
          'cassandra-l-4' 'nomerge' )

inputfile_with_linenumbers="$(mktemp)"
for method in ${methods[@]}; do

    inputfile="${statsfolder}/${method}-memsize.totalstats"
    outputfile="${outfolder}/${method}-memsize.totalstats"

    my_print ${method}
    ensure_file_exist ${inputfile}

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${inputfile} > ${inputfile_with_linenumbers}

    ymax_ins=`cat ${inputfile_with_linenumbers}  | awk '{if ($4 > max) max = $4;} END{print (max/60.0) * 1.05}'`
    ymax_comp=`cat ${inputfile_with_linenumbers} | awk '{if ($7 > max) max = $7;} END{print (max/60.0) * 1.05}'`
    ymax_io=`cat ${inputfile_with_linenumbers}   | awk '{if ($11 + $12 > max) max = $11 + $12;} END{print (max/60.0) * 1.05}'`
    ymax_gb=`cat ${inputfile_with_linenumbers}   | awk '{if ($13 + $14 > max) max = $13 + $14;} END{print (max/1024.0) * 1.05}'`

    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%s\" %s", $1, NR-1}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font 'Helvetica,22'
#    set ytics font 'Helvetica,22'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 32
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
    set yrange [0:${ymax_ins}]
    set ylabel '${ylabel_ins}'
    plot '${inputfile_with_linenumbers}' using (sec2min(\$4)) ls 1 notitle

    # Compaction Time
    set out '${outputfile}.compacttime.eps'
    set yrange [0:${ymax_comp}]
    set ylabel '${ylabel_comp}'
    plot '${inputfile_with_linenumbers}' using (sec2min(\$7)) ls 1 notitle

    # I/O Time
    set out '${outputfile}.iotime.eps'
    set yrange [0:${ymax_io}]
    set ylabel '${ylabel_io}'
    plot '${inputfile_with_linenumbers}' using (sec2min(\$11 + \$12)) ls 1 notitle

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set yrange [0:${ymax_gb}]
    set ylabel '${ylabel_gb}'
    plot '${inputfile_with_linenumbers}' using (mb2gb(\$13 + \$14)) ls 1 notitle

EOF
#==========================[ gnuplot embedded script ]============================

done

rm $inputfile_with_linenumbers


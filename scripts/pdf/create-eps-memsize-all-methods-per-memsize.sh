#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

memsizes=( '0128' '0256' '0512' '1024' '2048' )

inputfile_with_linenumbers="$(mktemp)"
for memsize in ${memsizes[@]}; do

    inputfile="${statsfolder}/allmethods-memsize-${memsize}.totalstats"
    outputfile="${outfolder}/allmethods-memsize-${memsize}"

    my_print "memsize $memsize"
    ensure_file_exist ${inputfile}

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; sub(/_/, " ", $1); printf "\"%s\" %s", $1, NR-1}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,18'
#    set ytics font 'Helvetica,18'
#    set xlabel font 'Helvetica,21'
#    set ylabel font 'Helvetica,21'
    set terminal postscript color enhanced eps "Helvetica" 26
    set xlabel ''
    set xtics (${xticklabels}) rotate by -45
    set yrange [0:] # start y from 0

    set grid ytics

    set size 0.8,1.0

    set xtics nomirror

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms
    set key invert

    set style line 1 lt 1 lw 2 lc rgb '${color}'
    set style line 2 lt 1 lw 2 lc rgb '${color2}'

    # Insertion time - Break down in IO / Mem
    set out '${outputfile}.totaltime.breakdown.eps'
    set ylabel '${ylabel_ins}'
    plot \
    '${inputfile_with_linenumbers}' using (sec2min(\$11 + \$12))         ls 1 title 'Disk I/O', \
    '${inputfile_with_linenumbers}' using (sec2min(\$4 - (\$11 + \$12))) ls 2 title 'Mem \& CPU' 

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

    # Total data transferred (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel '${ylabel_gb}'
    plot '${inputfile_with_linenumbers}' using (mb2gb(\$13 + \$14)) ls 1 notitle
EOF
#==========================[ gnuplot embedded script ]============================

done

rm $inputfile_with_linenumbers


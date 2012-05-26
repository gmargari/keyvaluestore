#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

files=( 'rangemerge' 'immediate' 'nomerge' \
        'geometric-p-2' 'geometric-p-3' 'geometric-p-4' \
        'geometric-r-2' 'geometric-r-3' 'geometric-r-4' \
        'cassandra-l-2' 'cassandra-l-3' 'cassandra-l-4' )

suffix="-pthrput-0-gthrput-20-gthreads-0-gsize-10"

my_print 

# find maximum number of runs, set ymax = 'max number of runs' + 1
ymaxall=0
for file in ${files[@]}; do
    if [ "$file" = "nomerge" ]; then
        continue
    fi

    inputfile=${statsfolder}/${file}${suffix}.log
    ensure_file_exist ${inputfile}

    newymax=`cat $inputfile | awk '{if($15>max) max=$15} END{print max}'`
    if [ "$newymax" -gt "$ymaxall" ]; then
        ymaxall=$newymax
    fi
done

difffile=$(mktemp)
for file in ${files[@]}; do
    inputfile=${statsfolder}/${file}${suffix}.log
    outputfile=${outfolder}/${file}${suffix}

    my_print $file
    ensure_file_exist ${inputfile}

    ymax_ins=`cat ${inputfile}  | awk '{if ($2 > max) max=$2}               END{print (max/60.0)*1.03}'`
    ymax_comp=`cat ${inputfile} | awk '{if ($5 > max) max=$5}               END{print (max/60.0)*1.03}'`
    ymax_io=`cat ${inputfile}   | awk '{if ($9 + $10 > max) max=$9 + $10}   END{print (max/60.0)*1.03}'`
    ymax_gb=`cat ${inputfile}   | awk '{if ($11 + $12 > max) max=$11 + $12} END{print (max/1024.0)*1.03}'`

    cat ${inputfile} | awk 'NR % 2 == 0 {printf "%d ", $1; for (i=2;i<=NF;i++) {printf "%d ", $(i) - prev[i]; prev[i] = $i;} print ""}' > ${difffile}
    ymax_gb_diff=`cat ${difffile}   | awk '{if ($11 + $12 > max) max=$11 + $12} END{print (max/1024.0)*1.03}'`

    if [ "$file" = "nomerge" ]; then
        # for nomerge, ymax is its own ymax
        ymax_runs=`cat ${inputfile} | awk '{if ($15 > max) max=$15} END{print max + 1}'`
    else
        # for nomerge, ymax is the global ymax (not including nomerge)
        ymax_runs=$(( $ymaxall + 1 ))
    fi

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 32
    set xlabel '${xlabel_datains_gb}'

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0
    prevx = 0

    #set grid ytics

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set yrange [0:${ymax_ins}]
    set ylabel '${ylabel_ins}'
    plot '${inputfile}' using (mb2gb(\$1)):(sec2min(\$2)) notitle with lines lw 7 lc rgb '${color}'

    # Compaction time
    set out '${outputfile}.compacttime.eps'
    set yrange [0:${ymax_comp}]
    set ylabel '${ylabel_comp}'
    plot '${inputfile}' using (mb2gb(\$1)):(sec2min(\$5)) notitle with lines lw 7 lc rgb '${color}'

    # I/O time
    set out '${outputfile}.iotime.eps'
    set yrange [0:${ymax_io}]
    set ylabel '${ylabel_io}'
    plot '${inputfile}' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) notitle with lines lw 7 lc rgb '${color}'

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set yrange [0:${ymax_gb}]
    set ylabel '${ylabel_gb}'
    plot '${inputfile}' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) notitle with lines lw 7 lc rgb '${color}'

    # Number of runs
    set out '${outputfile}.fragm.eps'
    set yrange [0:${ymax_runs}]
    set ylabel '${ylabel_runs}'
    plot '${inputfile}' using (mb2gb(\$1)):15 notitle with lines lw 7 lc rgb '${color}'

    # Data inserted DIFFERENTIAL (GB)
    set out '${outputfile}.gbtransferred.diff.eps'
    set yrange [0:${ymax_gb_diff}]
    set ylabel '${ylabel_gb_diff}'
    plot '${difffile}' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) notitle with linespoints pt 1 lw 7 ps 2 lc rgb '${color}'

EOF
#==========================[ gnuplot embedded script ]============================

done

#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

#==========================================================================================
# For each method, plot GB inserted vs:
#  - Insertion time
#  - IO time
#  - Compaction Time
#  - GB transferred
#  - Number of runs
#==========================================================================================

newinputfile="$(mktemp)"

files=( 'rangemerge' 'immediate' 'nomerge' \
        'geometric-p-2' 'geometric-p-3' 'geometric-p-4' \
        'geometric-r-2' 'geometric-r-3' 'geometric-r-4' \
        'cassandra-l-2' 'cassandra-l-3' 'cassandra-l-4' )

for suffix in "" "-full-putthrput-full-getthrput"; do

    # find maximum number of runs, set ymax = 'max number of runs' + 1
    ymaxall=0
    for file in ${files[@]}; do
        if [ "$file" = "nomerge" ]; then
            continue
        fi

        inputfile=${statsfolder}/${file}${prefix}.log
        ensure_file_exist ${inputfile}

        newymax=`cat $inputfile | awk '{if($15>max) max=$15} END{print max}'`
        if [ "$newymax" -gt "$ymaxall" ]; then
            ymaxall=$newymax
        fi
    done

    # for all files...
    for file in ${files[@]}; do
        inputfile=${statsfolder}/${file}${prefix}.log
        outputfile=${outfolder}/${file}${prefix}

        ensure_file_exist ${inputfile}

        ymax_ins=`cat ${inputfile}  | awk '{if ($2 > max) max=$2}               END{print (max/60.0)*1.03}'`
        ymax_comp=`cat ${inputfile} | awk '{if ($5 > max) max=$5}               END{print (max/60.0)*1.03}'`
        ymax_io=`cat ${inputfile}   | awk '{if ($9 + $10 > max) max=$9 + $10}   END{print (max/60.0)*1.03}'`
        ymax_gb=`cat ${inputfile}   | awk '{if ($11 + $12 > max) max=$11 + $12} END{print (max/1024.0)*1.03}'`

        if [ "$file" = "nomerge" ]; then
            # for nomerge, ymax is its own ymax
            ymax_runs=`cat ${inputfile} | awk '{if ($15 > max) max=$15} END{print max + 1}'`
        else
            # for nomerge, ymax is the global ymax (not including nomerge)
            ymax_runs=$(( $ymaxall + 1 ))
        fi

        # create a new file, adding for each <x,y> pair a new <x,prev_y> pair, for better visualization in plots:
        # x y          x y
        # 1 10         1 10
        # 2 20    ==>  2 10 (new point)
        # 3 10         2 20
        # 4 30         3 20 (new point)
        #              3 10
        #              4 10 (new point)
        #              4 30
        cat ${inputfile} | awk '{if (prev) print $1 prev; print $0; prev=""; for(i=2;i<=NF;i++) prev=prev" "$(i)}' > ${newinputfile}

        my_print $file

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 32
    set xlabel 'Data inserted (GB)'

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    #set grid ytics

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set yrange [0:${ymax_ins}]
    set ylabel "$ylabel_ins"
    plot '${newinputfile}' using (mb2gb(\$1)):(sec2min(\$2)) notitle with lines lw 7 lc rgb '$color'

    # Compaction time
    set out '${outputfile}.compacttime.eps'
    set yrange [0:${ymax_comp}]
    set ylabel "$ylabel_comp"
    plot '${newinputfile}' using (mb2gb(\$1)):(sec2min(\$5)) notitle with lines lw 7 lc rgb '$color'

    # I/O time
    set out '${outputfile}.iotime.eps'
    set yrange [0:${ymax_io}]
    set ylabel "$ylabel_io"
    plot '${newinputfile}' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) notitle with lines lw 7 lc rgb '$color'

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set yrange [0:${ymax_gb}]
    set ylabel "$ylabel_gb"
    plot '${newinputfile}' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) notitle with lines lw 7 lc rgb '$color'

    # Number of runs
    set out '${outputfile}.fragm.eps'
    set yrange [0:${ymax_runs}]
    set ylabel "$ylabel_runs"
    plot '${newinputfile}' using (mb2gb(\$1)):15 notitle with lines lw 7 lc rgb '$color'

EOF
#==========================[ gnuplot embedded script ]============================

    done

    rm $newinputfile

done

#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

files=(                             \
'workload-10GB-pthrput-raw.out'     \
'workload-10GB-pthrput-5000rps.out' \
'workload-10GB-pthrput-2500rps.out' \
'workload-10GB-pthrput-1000rps.out' \
)
windowsize=1

#========================================================
# average_using_sliding_window()
#========================================================
function average_using_sliding_window {
    local tmp="$(mktemp)"
    cat $1 | awk -v w=$windowsize \
      '{sumt += $2 - t[w]; \
        for (i = w; i >= 2; i--) {t[i] = t[i-1];} t[1] = $2;
        if (NR >= w) print $1, (sumt>0 ? sumt/w : 0)}' > $tmp
    mv $tmp $1
}

tmpfile=$(mktemp)
for file in ${files[@]}; do
    
    for windowsize in 1 5 10 15; do

        inputfile=${statsfolder}/cassandra/${file}
        outputfile=${outfolder}/cassandra.${file}.${windowsize}.eps

        my_print ${file} window = ${windowsize}
        ensure_file_exist ${inputfile}
        cat ${inputfile} | awk '$1=="[SCAN]," {if (++i > 6) printf "%.2f %f\n", $2/60000, $3/1000}' > ${tmpfile} # first 5 lines contain avg stats
        average_using_sliding_window ${tmpfile}

        xmin=-2
        xmax=`cat ${tmpfile} | awk '{if ($1 > max) max=$1;} END{print max+2}'`
        ymax=`cat ${tmpfile} | awk '{if ($2 > max) max=$2;} END{print max * 1.05}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font   'Helvetica,21'
#    set ytics font   'Helvetica,21'
#    set y2tics font  'Helvetica,21'
#    set xlabel font  'Helvetica,22'
#    set ylabel font  'Helvetica,22'
#    set y2label font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 20
    set xlabel '${xlabel_time}'
    set xrange [${xmin}:${xmax}]
    set yrange [0:${ymax}]

    # legend position
    set key top left

    set ylabel '${ylabel_glatency}'

    set out '${outputfile}'
    plot '${tmpfile}' using 1:2 with lines lw 4 lc rgb '${color}' notitle

EOF
#==========================[ gnuplot embedded script ]============================

    done
done

rm ${tmpfile}

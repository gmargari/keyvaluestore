#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi
        
statsfolder=$1
outfolder="$statsfolder/eps"
mkdir -p $outfolder

#========================================================
# my_print()
#========================================================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

#========================================================
# average_using_sliding_window()
#========================================================
function average_using_sliding_window {
    tmp="$(mktemp)"
    cat $1 | awk -v w=$windowsize \
      '{sumt += $2 - t[w]; suml += $3 - l[w]; \
        for (i = w; i >= 2; i--) {t[i] = t[i-1]; l[i] = l[i-1];} t[1] = $2; l[1] = $3; 
        if (NR >= w) print $1, (sumt>0 ? sumt/w : 0), (suml>0 ? suml/w : 0)}' > $tmp
    mv $tmp $1
}

#========================================================
# main script starts here
#========================================================

# average using a sliding of size 'windowsize'
windowsize=15
skip=5

get_stats_file1="$(mktemp)"
get_stats_file2="$(mktemp)"
get_stats_file3="$(mktemp)"
tmpfile="$(mktemp)"

for i in `seq 1 4`; do

    if [ $i == "1" ]; then
        file1="geometric-r-2-put-thrput-2500-mergebuf-512KB.stderr"
        file2="geometric-r-3-put-thrput-2500-mergebuf-512KB.stderr"
        file3="rangemerge-put-thrput-2500-mergebuf-512KB.stderr"
        outfile="log-rng-geom-put-thrput-2500-mergebuf-512KB"
    elif [ $i == "2" ]; then
        file1="geometric-r-2-put-thrput-0-mergebuf-512KB.stderr"
        file2="geometric-r-3-put-thrput-0-mergebuf-512KB.stderr"
        file3="rangemerge-put-thrput-0-mergebuf-512KB.stderr"
        outfile="log-rng-geom-put-thrput-0-mergebuf-512KB"
    elif [ $i == "3" ]; then
        file1="geometric-r-2-put-thrput-2500-mergebuf-4MB.stderr"
        file2="geometric-r-3-put-thrput-2500-mergebuf-4MB.stderr"
        file3="rangemerge-put-thrput-2500-mergebuf-4MB.stderr"
        outfile="log-rng-geom-put-thrput-2500-mergebuf-4MB"
    elif [ $i == "4" ]; then
        file1="geometric-r-2-put-thrput-0-mergebuf-4MB.stderr"
        file2="geometric-r-3-put-thrput-0-mergebuf-4MB.stderr"
        file3="rangemerge-put-thrput-0-mergebuf-4MB.stderr"
        outfile="log-rng-geom-put-thrput-0-mergebuf-4MB"
    fi

    title1="Logar. / Geom r=2"
    title2="Geometric r=3"
    title3="Rangemerge"

    linestyle1="lines lw 3 lt 4 lc rgb '#4CB04A'"
    linestyle2="lines lw 5 lt 3 lc rgb '#387DB8'"
    linestyle3="lines lw 4 lt 1 lc rgb '#E4191C'"

    inputfile1="${statsfolder}/${file1}"
    inputfile2="${statsfolder}/${file2}"
    inputfile3="${statsfolder}/${file3}"
    outputfile="${outfolder}/${outfile}"

    if [ ! -f "$inputfile1" ]; then echo "$file1 does not exist!"; continue; fi
    if [ ! -f "$inputfile2" ]; then echo "$file1 does not exist!"; continue; fi
    if [ ! -f "$inputfile3" ]; then echo "$file1 does not exist!"; continue; fi

    my_print $file1 $file2 $file3

    # get_stats rows: "[GET_STATS] <timestamp> <time_elapsed> <num_requests> <totallatency>
    cat $inputfile1 | awk '{if ($1=="[GET_STATS]") {print $2, ($3?$4/$3:0), ($4?$5/$4:0)}}' > $get_stats_file1
    cat $inputfile2 | awk '{if ($1=="[GET_STATS]") {print $2, ($3?$4/$3:0), ($4?$5/$4:0)}}' > $get_stats_file2
    cat $inputfile3 | awk '{if ($1=="[GET_STATS]") {print $2, ($3?$4/$3:0), ($4?$5/$4:0)}}' > $get_stats_file3

    # take average using a sliding window
    average_using_sliding_window $get_stats_file1
    average_using_sliding_window $get_stats_file2
    average_using_sliding_window $get_stats_file3

    latency_max=`cat $get_stats_file1 $get_stats_file2 $get_stats_file3 > $tmpfile; cat $tmpfile | awk '$3 > max {max=$3} END{print max}'`
    thrput_max=`cat $get_stats_file1 $get_stats_file2 $get_stats_file3 > $tmpfile; cat $tmpfile | awk '$2 > max {max=$2} END{print max}'`
    time_max=`cat $get_stats_file1 $get_stats_file2 $get_stats_file3 > $tmpfile; cat $tmpfile | awk '{max=$1} END{print max}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 24
    set xlabel 'Time (min)'
    set xrange [0:(${time_max}/60000)*1.05]

    # legend position
    set key top left
#    set key at 2.5,5.6

#    set size ratio 0.4

    ms_to_min(x) = x/60000

    set out '${outputfile}.latency.eps'
    set ylabel 'Get latency (ms)'
    set yrange [0:${latency_max}*1.05]
    plot \
    '${get_stats_file1}' using (ms_to_min(\$1)):3 every $skip with $linestyle1 title '$title1', \
    '${get_stats_file2}' using (ms_to_min(\$1)):3 every $skip with $linestyle2 title '$title2', \
    '${get_stats_file3}' using (ms_to_min(\$1)):3 every $skip with $linestyle3 title '$title3'


EOF
#==========================[ gnuplot embedded script ]============================


#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 24
    set xlabel 'Time (min)'
    set xrange [0:(${time_max}/60000)*1.05]

    # legend position
    set key bottom right
#    set key at 2.5,5.6

#    set size ratio 0.4

    ms_to_min(x) = x/60000

    set out '${outputfile}.thrput.eps'
    set ylabel 'Get throughput (req/s)'
    set yrange [0:${thrput_max}*1.05]
    plot \
    '${get_stats_file1}' using (ms_to_min(\$1)):2 every $skip with $linestyle1 title '$title1', \
    '${get_stats_file2}' using (ms_to_min(\$1)):2 every $skip with $linestyle2 title '$title2', \
    '${get_stats_file3}' using (ms_to_min(\$1)):2 every $skip with $linestyle3 title '$title3'

EOF
#==========================[ gnuplot embedded script ]============================

    rm $get_stats_file1 $get_stats_file2 $get_stats_file3 $tmpfile

done


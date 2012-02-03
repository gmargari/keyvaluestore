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
# For each method, plot GB inserted vs:
#  - Insertion time
#  - IO time
#  - Compaction Time
#  - GB transferred
#  - Number of runs
#==========================================================================================

inputfile_with_linenumbers="/tmp/gnuplottmp" # tmp file
newinputfile="/tmp/gnuplottmp2" # tmp file

color="#387DB8"

files=( 'rangemerge.log' 'immediate.log' 'nomerge.log' \
        'geometric-p-2.log' 'geometric-p-3.log' 'geometric-p-4.log' \
        'geometric-r-2.log' 'geometric-r-3.log' 'geometric-r-4.log' \
        'cassandra-l-2.log' 'cassandra-l-3.log' 'cassandra-l-4.log' )

# find maximum number of runs, set ymax = 'max number of runs' + 1
ymaxall=0
for i in `seq 0 $(( ${#files[*]} - 1 ))`; do
    file=${files[$i]}
    if [ "$file" = "nomerge.log" ]; then
        continue
    fi

    # check if file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    inputfile=${statsfolder}/$file
    newymax=`cat $inputfile | awk '{if($15>max) max=$15} END{print max}'`
    if [ "$newymax" -gt "$ymaxall" ]; then
        ymaxall=$newymax
    fi
done

# for all files...
for i in `seq 0 $(( ${#files[*]} - 1 ))`; do

    file=${files[$i]}
    inputfile=${statsfolder}/$file
    outputfile="`echo ${outfolder}/$file | awk '{sub(/\\.log/,\"\", $1); print $1}'`"  # remove suffix .log

    # check if file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    if [ "$file" = "nomerge.log" ]; then
        # for nomerge, ymax is its own ymax
        ymax=`cat ${inputfile} | awk '{if ($15 > max) max=$15} END{print max + 1}'`
    else
        # for nomerge, ymax is the global ymax (not including nomerge)
        ymax=$(( $ymaxall + 1 ))
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

    ylabel_ins='Insertion time (min)'
    ylabel_comp='Compaction time (min)'
    ylabel_io='I/O time (min)'
    ylabel_gb='Total data transferred (GB)'
    ylabel_runs='Number of disk files'

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
    set ylabel "$ylabel_ins"
    plot '${newinputfile}' using (mb2gb(\$1)):(sec2min(\$2)) notitle with lines lw 7 lc rgb '$color'

    # Compaction time
    set out '${outputfile}.compacttime.eps'
    set ylabel "$ylabel_comp"
    plot '${newinputfile}' using (mb2gb(\$1)):(sec2min(\$5)) notitle with lines lw 7 lc rgb '$color'

    # I/O time
    set out '${outputfile}.iotime.eps'
    set ylabel "$ylabel_io"
    plot '${newinputfile}' using (mb2gb(\$1)):(sec2min(\$9 + \$10)) notitle with lines lw 7 lc rgb '$color'

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel "$ylabel_gb"
    plot '${newinputfile}' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) notitle with lines lw 7 lc rgb '$color'

    set yrange [0:${ymax}]

    # Number of runs
    set out '${outputfile}.fragm.eps'
    set ylabel "$ylabel_runs"
    plot '${newinputfile}' using (mb2gb(\$1)):15 notitle with lines lw 7 lc rgb '$color'

EOF
#==========================[ gnuplot embedded script ]============================

done



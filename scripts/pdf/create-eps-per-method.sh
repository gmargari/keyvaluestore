#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1
outfolder="$statsfolder/eps"
inputfile_with_linenumbers="/tmp/gnuplottmp" # tmp file
newinputfile="/tmp/gnuplottmp2" # tmp file
mkdir -p $outfolder

#============================
# my_print()
#============================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

#==========================================================================================
# For each input file (same method with different parameter values) create a bar chart
#==========================================================================================

files=(  'rangemerge-memsize.totalstats' 'immediate-memsize.totalstats' 'nomerge-memsize.totalstats' \
         'geometric-p-4-memsize.totalstats' 'geometric-p-3-memsize.totalstats' 'geometric-p-2-memsize.totalstats' \
         'geometric-r-4-memsize.totalstats' 'geometric-r-3-memsize.totalstats' 'geometric-r-2-memsize.totalstats' \
         'cassandra-l-4-memsize.totalstats' 'cassandra-l-3-memsize.totalstats' 'cassandra-l-2-memsize.totalstats' \
         'allmethods.totalstats' 'allmethods-no-imm-no-rngmrg.totalstats'
         'rangemerge-blocksize.totalstats' 'rangemerge-flushmem.totalstats'  \
         )

xlabels=( 'Memory size (MB)' 'Memory size (MB)' 'Memory size (MB)' \
          'Memory size (MB)' 'Memory size (MB)' 'Memory size (MB)' \
          'Memory size (MB)' 'Memory size (MB)' 'Memory size (MB)' \
          'Memory size (MB)' 'Memory size (MB)' 'Memory size (MB)' \
          ' ' ' ' \
          'Block size (MB)' 'Flushmem size (MB)' \
         )

# check arrays 'files' and 'xlabels' have the same number of elements
if [ ${#files[*]} -ne ${#xlabels[*]} ]; then
    echo "Error: different array size of 'files' (${#files[*]}) and 'xlabels' (${#xlabels[*]})" >&2;
    exit 1;
fi

# for each file in array 'files' create a number of eps
for i in `seq 0 $(( ${#xlabels[*]} - 1 ))`; do

    inputfile="${statsfolder}/${files[$i]}"
    outputfile="${outfolder}/${files[$i]}"
    xlabel=${xlabels[$i]};

    # check that file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", NR, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%s\" %s", $1, NR}'`

    my_print ${files[$i]}

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    # mb_ins(1) Ttotal(2) Tcompac(3) Tput(4) Tmerge(5) Tfree(6) Tcmrest(7) Tmem(8) Tread(9) Twrite(10) mb_read(11) mb_writ(12) reads(13) writes(14) runs(15)

    set xtics (${xticklabels})
    set yrange [0:] # start y from 0
    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,22'
#    set ytics font 'Helvetica,22'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 24
    set xlabel '${xlabel}'

    mb2gb(x) = x/1024.0

    set boxwidth 0.50

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    #set ylabel 'Total data read/written (GB)'
    plot '${inputfile_with_linenumbers}' using 1:(mb2gb(\$13 + \$14)) notitle with boxes fill pattern 3 lc rgb 'black'

    # I/O Time
    set out '${outputfile}.iotime.eps'
    #set ylabel 'I/O time (s)'
    plot '${inputfile_with_linenumbers}' using 1:(\$11 + \$12) notitle with boxes fill pattern 3 lc rgb 'black'

    # Compaction Time
    set out '${outputfile}.compacttime.eps'
    #set ylabel 'Compaction time (s)' # "Tmerge", contains IO + memcmp + memcpy
    plot '${inputfile_with_linenumbers}' using 1:7 notitle with boxes fill pattern 3 lc rgb 'black'

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    #set ylabel 'Insertion time (s)'
    plot '${inputfile_with_linenumbers}' using 1:4 notitle with boxes fill pattern 3 lc rgb 'black'

EOF
#==========================[ gnuplot embedded script ]============================

done



#==========================================================================================
# For each input file (a specific method) plot GB inserted vs current number of runs
#==========================================================================================

files=( 'rangemerge.log' 'immediate.log' 'nomerge.log' \
        'geometric-p-2.log' 'geometric-p-3.log' 'geometric-p-4.log' \
        'geometric-r-2.log' 'geometric-r-3.log' 'geometric-r-4.log' \
        'cassandra-l-2.log' 'cassandra-l-3.log' 'cassandra-l-4.log' )

# find maximum number of runs, set ymax = 'max number of runs' + 1
#cd ${statsfolder}
#ymaxall=`cat ${files[*]} | awk '{if ($15 > max) max=$15} END{print max}'`
#cd -
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


for i in `seq 0 $(( ${#files[*]} - 1 ))`; do

    file=${files[$i]}
    inputfile=${statsfolder}/$file
    outputfile="`echo ${outfolder}/$file | awk '{sub(/\\.log/,\"\", $1); print $1}'`"  # remove suffix .log

    # check if file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    if [ "$file" = "nomerge.log" ]; then
        ymax=`cat ${inputfile} | awk '{if ($15 > max) max=$15} END{print max + 1}'`
    else
        ymax=$(( $ymaxall + 1 ))
    fi

    my_print $file

# mb_ins(1) Ttotal(2) Tcompac(3) Tput(4) Tmerge(5) Tfree(6) Tcmrest(7) Tmem(8) Tread(9) Twrite(10) mb_read(11) mb_writ(12) reads(13) writes(14) runs(15)


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

skippoints=1

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 26
    set xlabel 'Data inserted (GB)'

    mb2gb(x) = x/1024.0

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set ylabel 'Insertion time (s)'
    plot '${newinputfile}' using (mb2gb(\$1)):2 every ${skippoints} notitle with lines lw 2 lc rgb "black"

    # Compaction time
    set out '${outputfile}.compacttime.eps'
    set ylabel 'Compaction time (s)'
    plot '${newinputfile}' using (mb2gb(\$1)):5 every ${skippoints} notitle with lines lw 2 lc rgb "black"

    # I/O time
    set out '${outputfile}.iotime.eps'
    set ylabel 'I/O time (s)'
    plot '${newinputfile}' using (mb2gb(\$1)):(\$9 + \$10) every ${skippoints} notitle with lines lw 2 lc rgb "black"

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel 'Total data transferred (GB)'
    plot '${newinputfile}' using (mb2gb(\$1)):(mb2gb(\$11 + \$12)) every ${skippoints} notitle with lines lw 2 lc rgb "black"

    set yrange [0:${ymax}]

    set out '${outputfile}.fragm.eps'
    set ylabel 'Number of disk files'
    plot '${newinputfile}' using (mb2gb(\$1)):15 notitle with lines lw 2 lc rgb "black"

EOF
#==========================[ gnuplot embedded script ]============================

done



exit


#==========================================================================================
# Logarithmic: Number of runs vs range get time
#==========================================================================================

file="logarithmic.search.log"
inputfile="${statsfolder}/$file"
outputfile="`echo ${outfolder}/$file | awk '{sub(/\\.log/,\"\", $1); print $1}'`"  # remove suffix .log

ymax=`cat $inputfile | awk '{if ($11 > max) max=$11;} END{print max}'`
y2max=`cat $inputfile | awk '{if ($12 > max) max=$12;} END{print max}'`

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

#    set xtics font  'Helvetica,21'
#    set ytics font  'Helvetica,21'
#    set y2tics font  'Helvetica,21'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
#    set y2label font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 20
    set xlabel 'Data inserted (GB)'
    set x2label 'Read cost' font 'Helvetica Bold'

    # legend position
    #set key top left
    set key at 2.5,5.6

    set yrange [0:${ymax}+1]
    set ylabel 'Number of disk files'
    set ytic nomirror
    set y2range [0:${y2max}+1]
    set y2label 'Range get latency (ms)'
    set y2tic nomirror

    mb2gb(x) = x/1024.0

    set out '${outputfile}.eps'

    plot \
    '${inputfile}' using (mb2gb(\$1)):11 title 'Number of disk files' with lines lw 2 lc rgb "black", \
    '${inputfile}' using (mb2gb(\$1)):(( \$12 / ${y2max} ) * (${ymax}))title 'Range get latency' with lines lw 2 lc rgb "black"

EOF
#==========================[ gnuplot embedded script ]============================



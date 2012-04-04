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
# For each method, and a specific parameter with different values, create a bar chart for:
#  - Insertion time
#  - IO time
#  - Compaction Time 
#  - GB transferred
#==========================================================================================

inputfile_with_linenumbers="/tmp/gnuplottmp" # tmp file
newinputfile="/tmp/gnuplottmp2" # tmp file

color="#387DB8"

files=( 'rangemerge-blocksize.totalstats' 'rangemerge-flushmem.totalstats' )
xlabels=( 'Block size (MB)' 'Flushmem size (MB)' )

# check arrays 'files' and 'xlabels' have the same number of elements
if [ ${#files[*]} -ne ${#xlabels[*]} ]; then
    echo "Error: different array size of 'files' (${#files[*]}) and 'xlabels' (${#xlabels[*]})" >&2;
    exit 1;
fi

# for each file in array 'files' create a number of eps
for i in `seq 0 $(( ${#xlabels[*]} - 1 ))`; do

    file=${files[$i]}
    inputfile="${statsfolder}/$file"
    outputfile="${outfolder}/$file"
    xlabel=${xlabels[$i]};

    # check that file exists
    if [ ! -f ${inputfile} ]; then echo "Error: file '${inputfile}' does not exist" >&2; continue; fi

    # create a new file, in which we prepend lines of original file with line numbers (will be used to place bars in eps)
    awk '{if (NF>1) printf "%s %s\n", ++i, $0}' ${inputfile} > ${inputfile_with_linenumbers}
    lines=`cat ${inputfile_with_linenumbers} | wc -l`
    xticklabels=`cat ${inputfile} | awk '{if (NR>1) printf ", "; printf "\"%s\" %s", $1, NR-1}'`

    ylabel_ins='Insertion time (min)'
    ylabel_comp='Compaction time (min)'
    ylabel_io='I/O time (min)'
    ylabel_gb='Total data transferred (GB)'

    my_print $file

#==========================[ gnuplot embedded script ]============================
gnuplot << EOF

    # mb_ins(1) Ttotal(2) Tcompac(3) Tput(4) Tmerge(5) Tfree(6) Tcmrest(7) Tmem(8) Tread(9) Twrite(10) mb_read(11) mb_writ(12) reads(13) writes(14) runs(15)

    set xtics (${xticklabels})
    set yrange [0:] # start y from 0
#    set xrange [0:${lines}+1]
#    set xtics font 'Helvetica,22'
#    set ytics font 'Helvetica,22'
#    set xlabel font 'Helvetica,22'
#    set ylabel font 'Helvetica,22'
    set terminal postscript color enhanced eps "Helvetica" 22
    set xlabel '${xlabel}'

    mb2gb(x) = x/1024.0
    sec2min(x) = x/60.0

    set boxwidth 0.50
    set style fill solid border 0
    set style histogram rowstacked
    set style data histograms
    set key invert

    set grid ytics

    set style line 1 lt 1 lw 2 lc rgb "$color"

    # Insertion time
    set out '${outputfile}.totaltime.eps'
    set ylabel "$ylabel_ins"
    plot '${inputfile_with_linenumbers}' using (sec2min(\$4)) ls 1 notitle

    # Compaction Time
    set out '${outputfile}.compacttime.eps'
    set ylabel "$ylabel_comp"
    plot '${inputfile_with_linenumbers}' using (sec2min(\$7)) ls 1 notitle

    # I/O Time
    set out '${outputfile}.iotime.eps'
    set ylabel "$ylabel_io"
    plot '${inputfile_with_linenumbers}' using (sec2min(\$11 + \$12)) ls 1 notitle

    # Data inserted (GB)
    set out '${outputfile}.gbtransferred.eps'
    set ylabel "$ylabel_gb"
    plot '${inputfile_with_linenumbers}' using (mb2gb(\$13 + \$14)) ls 1 notitle

EOF
#==========================[ gnuplot embedded script ]============================

done


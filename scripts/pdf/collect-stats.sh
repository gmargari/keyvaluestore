#!/bin/bash

#============================
# my_print()
#============================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

#============================
# create_log_files()
#============================
create_log_files() {

    my_print "create_log_files()"

    # just remove all lines starting with "#", and keep only the stats refering
    # to the first "keepbytes" bytes inserted
    keepbytes=200000 # keep all

    for f in `ls -1 ${statsfolder}/*.stats 2> /dev/null`; do
        OUTFILE=`echo $f | awk '{sub(/\.stats/,".log", $1); print $1}'`
        cat $f  |
          awk -v kb=$keepbytes '$1 != "#" && $1 < kb { for (i=1; i<=NF; i++) printf "%s ", $(i); print ""}' > $OUTFILE;
    done

    # fix rangemerge stats, so it always contain 1 run
    for f in `ls -1 ${statsfolder}/rangemerge*.stats 2> /dev/null`; do
        OUTFILE=`echo $f | awk '{sub(/\.stats/,".log", $1); print $1}'`
        cat $f |
          awk -v kb=$keepbytes '$1 != "#" && $1 < kb { $15 = 1; for (i=1; i<=NF; i++) printf "%s ", $(i); print ""}' > $OUTFILE;
    done
}

#============================
# collect_allmethods_stats()
#============================
collect_allmethods_stats() {

    my_print "collect_allmethods_stats()"

    files=( 'immediate.stats' 'rangemerge.stats' 'geometric-p-2.stats' 'geometric-p-3.stats' 'geometric-p-4.stats' \
            'geometric-r-4.stats' 'geometric-r-3.stats' 'geometric-r-2.stats' \
            'cassandra-l-4.stats' 'cassandra-l-3.stats' 'cassandra-l-2.stats' \
            'nomerge.stats' )
    labels=( 'Immediate' 'Rangemerge' 'Geom_{p=2}' 'Geom_{p=3}' 'Geom_{p=4}' \
             'Geom_{r=4}' 'Geom_{r=3}' 'Geom_{r=2}' \
             'Cass_{l=4}' 'Cass_{l=3}' 'Cass_{l=2}' \
             'Nomerge' )

    if [ ${#files[*]} -ne ${#labels[*]} ]; then
        echo "Error: different array size of 'files' (${#files[*]}) and 'labels' (${#labels[*]})" >&2
        exit 1;
    fi

    outfile=${statsfolder}/allmethods.totalstats
    for i in `seq 0 $(( ${#labels[*]} - 1 ))`; do
        file=${statsfolder}/${files[$i]};
        label=${labels[$i]};

        if [ ! -f ${file} ]; then echo "Error: file '${file}' does not exist" >&2; continue; fi

        tail -n 1 $file | awk -v label="$label" '{printf "%s %s\n", label, $0}';
    done > $outfile

    if [ "`cat $outfile | wc -l`" == "0" ]; then # if file empty, delete it
        rm $outfile
    fi

    my_print "collect_allmethods_stats() no-imm no-rangemerge"

    outfile=${statsfolder}/allmethods-no-imm-no-rngmrg.totalstats
    for i in `seq 0 $(( ${#labels[*]} - 1 ))`; do
        file=${statsfolder}/${files[$i]};
        label=${labels[$i]};

        if [[ "${files[$i]}" =~ ^immediate ]]; then
            continue
        elif [[ "${files[$i]}" =~ ^rangemerge ]]; then
            continue
        fi

        if [ ! -f ${file} ]; then echo "Error: file '${file}' does not exist" >&2; continue; fi

        tail -n 1 $file | awk -v label="$label" '{printf "%s %s\n", label, $0}';
    done > $outfile

    if [ "`cat $outfile | wc -l`" == "0" ]; then # if file empty, delete it
        rm $outfile
    fi
}

#============================
# collect_stats()
#============================
collect_stats() {

    my_print "collect_stats() $prefix $field"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-*.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/$prefix-*.stats; do
        grep "# ${field}:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -n > ${statsfolder}/$prefix.totalstats

    # in case of rangemerge-blocksize, if we have a row for blocksize = "inf",
    # remove it from first row (since file is sorted by blocksize) and
    # move it at last row
    if [ "`cat ${statsfolder}/$prefix.totalstats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    if [ $field ==  "rngmerge_block_size" ]; then
        cat ${statsfolder}/$prefix.totalstats |
          awk '{if ($1 == "inf") {firstrow=$0; bs0=1} else print $0} END{if(bs0) print firstrow}' > ${statsfolder}/$prefix.totalstats.del
          mv ${statsfolder}/$prefix.totalstats.del ${statsfolder}/$prefix.totalstats
    fi
}



#==================================
# collect_stats_per_memsize()
#==================================
collect_stats_per_memsize() {

    my_print "collect_stats_per_memory_size()"

    files=( 'immediate' 'rangemerge' 'geometric-p-2' 'geometric-r-3' 'geometric-r-2' \
            'cassandra-l-4' 'cassandra-l-3' 'cassandra-l-2' 'nomerge' )
    labels=( 'Immediate' 'Rangemerge' 'Geom_{p=2}' 'Geom_{r=3}' 'Geom_{r=2}'
             'Cass_{l=4}' 'Cass_{l=3}' 'Cass_{l=2}' 'Nomerge' )

    memsizes=( '0128' '0256' '0512' '1024' '2048' )

    if [ ${#files[*]} -ne ${#labels[*]} ]; then
        echo "Error: different array size of 'files' (${#files[*]}) and 'labels' (${#labels[*]})" >&2
        exit 1;
    fi

    for i in `seq 0 $(( ${#memsizes[*]} - 1 ))`; do
        memsize=${memsizes[$i]};
        outfile=${statsfolder}/allmethods-memsize-${memsize}.totalstats
        for j in `seq 0 $(( ${#labels[*]} - 1 ))`; do
            file=${statsfolder}/${files[$j]}-memsize-$memsize.stats;
            label=${labels[$j]};

            if [ ! -f ${file} ]; then echo "Error: file '${file}' does not exist" >&2; continue; fi

            tail -n 1 $file | awk -v label="$label" '{printf "%s %s\n", label, $0}';
        done > $outfile

        if [ "`cat $outfile | wc -l`" == "0" ]; then # if file empty, delete it
            rm $outfile
        fi
    done
}

#============================
# main script starts here
#============================

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1

# create *.log files that will be used for plots
create_log_files

# create files 'allmethods.totalstats' and 'allmethods-no-imm-no-rngmerge.totalstats'
collect_allmethods_stats
#exit


# create allmethods-memsize-*.totalstats files that contain stats for all methods for a specific memsize
collect_stats_per_memsize

prefix="rangemerge-blocksize"
field="rngmerge_block_size"
collect_stats

prefix="rangemerge-flushmem"
field="rngmerge_flushmem_size"
collect_stats

prefix="rangemerge-memsize"
field="memory_size"
collect_stats

prefix="immediate-memsize"
field="memory_size"
collect_stats

prefix="nomerge-memsize"
field="memory_size"
collect_stats

prefix="geometric-r-4-memsize"
field="memory_size"
collect_stats

prefix="geometric-r-3-memsize"
field="memory_size"
collect_stats

prefix="geometric-r-2-memsize"
field="memory_size"
collect_stats

prefix="geometric-p-4-memsize"
field="memory_size"
collect_stats

prefix="geometric-p-3-memsize"
field="memory_size"
collect_stats

prefix="geometric-p-2-memsize"
field="memory_size"
collect_stats

prefix="cassandra-l-2-memsize"
field="memory_size"
collect_stats

prefix="cassandra-l-3-memsize"
field="memory_size"
collect_stats

prefix="cassandra-l-4-memsize"
field="memory_size"
collect_stats


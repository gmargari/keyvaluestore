#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

statsfolder=$1

#========================================================
# my_print()
#========================================================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

#========================================================
# create_log_files()
#========================================================
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

#========================================================
# collect_allmethods_stats()
#========================================================
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
}

#========================================================
# collect_memsize_stats_per_method()
#========================================================
collect_memsize_stats_per_method() {

    my_print "collect_memsize_stats_per_method() $prefix memstore_size"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-????.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/$prefix-????.stats; do
        grep "# memstore_size:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -n > ${statsfolder}/$prefix.totalstats

    if [ "`cat ${statsfolder}/$prefix.totalstats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi
}

#========================================================
# collect_rngmerge_blocksize_stats()
#========================================================
collect_rngmerge_blocksize_stats() {

    my_print "collect_rngmerge_blocksize_stats()"

    prefix="rangemerge-blocksize"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-????.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/$prefix-????.stats; do
        grep "# rngmerge_block_size:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -n > ${statsfolder}/$prefix.totalstats

    if [ "`cat ${statsfolder}/$prefix.totalstats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    # if we have a row for blocksize = "inf", remove it from first row 
    # (since file is sorted by blocksize) and move it at last row
    cat ${statsfolder}/$prefix.totalstats |
      awk '{if ($1 == "inf") {firstrow=$0; bs0=1} else print $0} END{if(bs0) print firstrow}' > ${statsfolder}/$prefix.totalstats.del
      mv ${statsfolder}/$prefix.totalstats.del ${statsfolder}/$prefix.totalstats
}

#========================================================
# collect_rngmerge_flushmem_stats()
#========================================================
collect_rngmerge_flushmem_stats() {

    my_print "collect_rngmerge_flushmem_stats()"

    prefix="rangemerge-flushmem"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-????.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/$prefix-????.stats; do
        grep "# rngmerge_flushmem_size:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -n > ${statsfolder}/$prefix.totalstats

    if [ "`cat ${statsfolder}/$prefix.totalstats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi
}

#==============================================================
# collect_stats_per_memstore_size()
#==============================================================
collect_stats_per_memstore_size() {

    my_print "collect_stats_per_memstore_size()"

    files=( 'immediate' 'rangemerge' 'geometric-p-2' 'geometric-r-3' 'geometric-r-2' \
            'cassandra-l-2' 'cassandra-l-4' 'nomerge' )
    labels=( 'Immediate' 'Rangemerge' 'Geom_{p=2}' 'Geom_{r=3}' 'Geom_{r=2}'
             'Cass_{l=2}' 'Cass_{l=4}' 'Nomerge' )

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

#========================================================
# collect_ordered_keys_stats()
#========================================================
collect_ordered_keys_stats() {

    my_print "collect_ordered_keys_stats() $prefix"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-???.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/${prefix}-???.stats; do
        grep "# ordered_prob:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -rn > ${statsfolder}/${prefix}.totalstats
}

#========================================================
# collect_zipf_keys_stats()
#========================================================
collect_zipf_keys_stats() {

    my_print "collect_zipf_keys_stats() $prefix"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-???.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/${prefix}-???.stats; do
        grep "# zipf_parameter:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -n > ${statsfolder}/${prefix}.totalstats
}

#========================================================
# main script starts here
#========================================================

# create *.log files that will be used for plots
create_log_files

# create files 'allmethods.totalstats' and 'allmethods-no-imm-no-rngmerge.totalstats'
collect_allmethods_stats

# create allmethods-memsize-*.totalstats files that contain stats for all methods for a specific memsize
collect_stats_per_memstore_size

collect_rngmerge_blocksize_stats

collect_rngmerge_flushmem_stats

prefix="rangemerge-memsize"
collect_memsize_stats_per_method

prefix="immediate-memsize"
collect_memsize_stats_per_method

prefix="nomerge-memsize"
collect_memsize_stats_per_method

prefix="geometric-r-4-memsize"
collect_memsize_stats_per_method

prefix="geometric-r-3-memsize"
collect_memsize_stats_per_method

prefix="geometric-r-2-memsize"
collect_memsize_stats_per_method

prefix="geometric-p-4-memsize"
collect_memsize_stats_per_method

prefix="geometric-p-3-memsize"
collect_memsize_stats_per_method

prefix="geometric-p-2-memsize"
collect_memsize_stats_per_method

prefix="cassandra-l-2-memsize"
collect_memsize_stats_per_method

prefix="cassandra-l-3-memsize"
collect_memsize_stats_per_method

prefix="cassandra-l-4-memsize"
collect_memsize_stats_per_method

prefix="geometric-p-2-ord-prob"
collect_ordered_keys_stats

prefix="geometric-p-3-ord-prob"
collect_ordered_keys_stats

prefix="geometric-p-4-ord-prob"
collect_ordered_keys_stats

prefix="geometric-r-2-ord-prob"
collect_ordered_keys_stats

prefix="geometric-r-3-ord-prob"
collect_ordered_keys_stats

prefix="geometric-r-4-ord-prob"
collect_ordered_keys_stats

prefix="cassandra-l-2-ord-prob"
collect_ordered_keys_stats

prefix="cassandra-l-3-ord-prob"
collect_ordered_keys_stats

prefix="cassandra-l-4-ord-prob"
collect_ordered_keys_stats

prefix="nomerge-ord-prob"
collect_ordered_keys_stats

prefix="rangemerge-ord-prob"
collect_ordered_keys_stats

prefix="immediate-ord-prob"
collect_ordered_keys_stats

prefix="rangemerge-blocksize-0064-ord-prob"
collect_ordered_keys_stats

prefix="rangemerge-blocksize-0128-ord-prob"
collect_ordered_keys_stats

prefix="rangemerge-blocksize-0256-ord-prob"
collect_ordered_keys_stats

prefix="rangemerge-zipf_a"
collect_zipf_keys_stats

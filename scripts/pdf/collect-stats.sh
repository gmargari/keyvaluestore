#!/bin/bash

curdir=`dirname $0`
source $curdir/include-script.sh
source $curdir/include-colors-titles.sh

statsfolder=$1

#========================================================
# create_log_files()
#========================================================
create_log_files() {
    my_print "${FUNCNAME}()"

    # just remove all lines starting with "#", and keep only the stats refering
    # to the first "keepbytes" bytes inserted
    keepbytes=1000000 # keep all

    for f in `ls -1 ${statsfolder}/*.stats 2> /dev/null`; do
        OUTFILE=`echo $f | awk '{sub(/\.stats/,".log", $1); print $1}'`
        cat $f  |
          awk -v kb=$keepbytes '$1 != "#" && $1 < kb { for (i=1; i<=NF; i++) printf "%s ", $(i); print ""}' > $OUTFILE;
    done

    # fix rangemerge stats, so it always contain 1 run
    for f in `ls -1 ${statsfolder}/rangemerge*.stats 2> /dev/null`; do
        OUTFILE=`echo $f | awk '{sub(/\.stats/,".log", $1); print $1}'`
        cat $f |
          awk -v kb=$keepbytes '$1 != "#" && $1 < kb { if ($15>1) $15=1; for (i=1; i<=NF; i++) printf "%s ", $(i); print ""}' > $OUTFILE;
    done
}

#========================================================
# collect_memsize_stats_per_method()
#========================================================
collect_memsize_stats_per_method() {
    my_print "${FUNCNAME}() $prefix memstore_size"

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
    my_print "${FUNCNAME}()"

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
    my_print "${FUNCNAME}()"

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
# collect_stats_per_memsize()
#==============================================================
collect_stats_per_memsize() {
    my_print "${FUNCNAME}()"

    files=( 'immediate' 'rangemerge' 'geometric-p-2' 'geometric-r-3' 'geometric-r-2' \
            'cassandra-l-4' 'nomerge' )
    labels=( 'Remerge' 'Rangemerge' 'Geom_p=2' 'Geom_r=3' "Geom_r=2"
             'SMA_k=4' 'Nomerge' )

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

            ensure_file_exist $file

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
    my_print "${FUNCNAME}() $prefix"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-???.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix.* file"
        return
    fi

    for f in ${statsfolder}/${prefix}-???.stats; do
        grep "# ordered_prob:" $f | awk '{printf "%s ", (1-$3)}' && tail -n 1 $f;
    done | sort -rn > ${statsfolder}/${prefix}.totalstats
}

#========================================================
# collect_zipf_keys_stats()
#========================================================
collect_zipf_keys_stats() {
    my_print "${FUNCNAME}() $prefix"

    # if no files exist return, instead of creating an empty file
    if [ "`cat ${statsfolder}/$prefix-???-unique.stats 2> /dev/null | wc -l`" == "0" ]; then
        echo "Error: no ${statsfolder}/$prefix-???-unique.stats file"
        return
    fi

    for f in ${statsfolder}/${prefix}-???-unique.stats; do
        grep "# zipf_parameter:" $f | awk '{printf "%s ", $3}' && tail -n 1 $f;
    done | sort -n > ${statsfolder}/${prefix}.totalstats
}

#========================================================
# collect_stats_numthreads()
#========================================================
collect_stats_per_numthreads() {
    my_print "${FUNCNAME}()"

    files=(  'immediate' 'rangemerge' 'geometric-r-3'  'geometric-r-2' 'cassandra-l-4'  'geometric-p-2' )
    numthreads=( '1' '2' '5' '10' )
    # percentile = 0 -> average time
    percentiles=( "0" "0.90" "0.99" "0.999" "1" )
    putthrput=2500
    getthreads=1
    getthrput=20
    getsize=10

    tmpfile2="$(mktemp)"
    tmpfile="$(mktemp)"
    for i in `seq 0 $(( ${#files[*]} - 1 ))`; do
        outfile=${statsfolder}/${files[$i]}-getthreads.totalstats

        for getthreads in ${numthreads[@]}; do
            getthrput=$(( 20 / $getthreads ))
            file=${statsfolder}/${files[$i]}-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}.stderr

            ensure_file_exist $file

            keep_get_stats $file | sort -n > $tmpfile2
            average_using_sliding_window $tmpfile2
            cat $tmpfile2 | sort -k 3 -n > $tmpfile

            # compute and print percentiles of latencies
            echo $getthreads | awk '{printf "%-20s ", $1}'
            lines=`cat $tmpfile | wc -l`
            for p in ${percentiles[@]}; do
                if [ $p == "0" ]; then
                    cat $tmpfile | awk '{s+=$3; n++} END{printf "%5.1f ", s/n}'
                else
                    cat $tmpfile | awk -v p=$p -v lines=$lines 'NR >= p * lines {printf "%5.1f ", $3; exit}'
                fi
            done

            # print also total time
            file=${statsfolder}/${files[$i]}-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}.stats
            tail -n 1 $file | awk '{printf "%10d", $2}'

            echo ""

        done > $outfile

        if [ "`cat $outfile | wc -l`" == "0" ]; then # if file empty, delete it
            rm $outfile
        fi
    done

    rm $tmpfile $tmpfile2
}

#========================================================
# collect_stats_per_putthruput()
#========================================================
collect_stats_per_putthruput() {
    my_print "${FUNCNAME}()"

    files=(  'immediate' 'rangemerge' 'geometric-r-3'  'geometric-r-2' 'cassandra-l-4'  'geometric-p-2' )
    putthrputs=( '1000' '2500' '5000' '10000' '20000' '40000' )
    # percentile = 0 -> average time
    percentiles=( "0" "0.90" "0.99" "0.999" "1" )
    putthrput=2500
    getthreads=1
    getthrput=20
    getsize=10

    tmpfile2="$(mktemp)"
    tmpfile="$(mktemp)"
    for i in `seq 0 $(( ${#files[*]} - 1 ))`; do
        outfile=${statsfolder}/${files[$i]}-putthrput.totalstats

        for putthrput in ${putthrputs[@]}; do
            file=${statsfolder}/${files[$i]}-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}.stderr

            ensure_file_exist $file

            keep_get_stats $file | sort -n > $tmpfile2
            average_using_sliding_window $tmpfile2
            cat $tmpfile2 | sort -k 3 -n > $tmpfile

            # compute and print percentiles of latencies
            echo $putthrput | awk '{printf "%-20s ", $1}'
            lines=`cat $tmpfile | wc -l`
            for p in ${percentiles[@]}; do
                if [ $p == "0" ]; then
                    cat $tmpfile | awk '{s+=$3; n++} END{printf "%5.1f ", s/n}'
                else
                    cat $tmpfile | awk -v p=$p -v lines=$lines 'NR >= p * lines {printf "%5.1f ", $3; exit}'
                fi
            done

            # print also total time
            file=${statsfolder}/${files[$i]}-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}.stats
            tail -n 1 $file | awk '{printf "%10d", $2}'

            echo ""

        done > $outfile

        if [ "`cat $outfile | wc -l`" == "0" ]; then # if file empty, delete it
            rm $outfile
        fi
    done

    rm $tmpfile $tmpfile2
}

#========================================================
# collect_stats_per_getsize()
#========================================================
collect_stats_per_getsize() {
    my_print "${FUNCNAME}()"

    files=(  'immediate' 'rangemerge' 'geometric-r-3'  'geometric-r-2' 'cassandra-l-4'  'geometric-p-2' )
    getsizes=( '1' '10' '100' '1000' '10000' '100000' )
    # percentile = 0 -> average time
    percentiles=( "0" "0.90" "0.99" "0.999" "1" )
    putthrput=2500
    getthreads=1
    getthrput=20
    getsize=10

    tmpfile2="$(mktemp)"
    tmpfile="$(mktemp)"
    for i in `seq 0 $(( ${#files[*]} - 1 ))`; do
        outfile=${statsfolder}/${files[$i]}-getsizes.totalstats

        for getsize in ${getsizes[@]}; do
            file=${statsfolder}/${files[$i]}-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}.stderr
            ensure_file_exist $file
            keep_get_stats $file | sort -n > $tmpfile2
            average_using_sliding_window $tmpfile2
            cat $tmpfile2 | sort -k 3 -n > $tmpfile

            # compute and print percentiles of latencies
            echo $getsize | awk '{printf "%-20s ", $1}'
            lines=`cat $tmpfile | wc -l`
            for p in ${percentiles[@]}; do
                if [ $p == "0" ]; then
                    cat $tmpfile | awk '{s+=$3; n++} END{printf "%5.1f ", s/n}'
                else
                    cat $tmpfile | awk -v p=$p -v lines=$lines 'NR >= p * lines {printf "%5.1f ", $3; exit}'
                fi
            done

            # print also total time
            file=${statsfolder}/${files[$i]}-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}.stats
            tail -n 1 $file | awk '{printf "%10d", $2}'

            echo ""

        done > $outfile

        if [ "`cat $outfile | wc -l`" == "0" ]; then # if file empty, delete it
            rm $outfile
        fi
    done

    rm $tmpfile $tmpfile2
}

#========================================================
# main script starts here
#========================================================

create_log_files

collect_stats_per_memsize

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
prefix="immediate-zipf_a"
collect_zipf_keys_stats
prefix="nomerge-zipf_a"
collect_zipf_keys_stats
prefix="geometric-r-2-zipf_a"
collect_zipf_keys_stats
prefix="geometric-r-3-zipf_a"
collect_zipf_keys_stats
prefix="geometric-p-2-zipf_a"
collect_zipf_keys_stats
prefix="cassandra-l-4-zipf_a"
collect_zipf_keys_stats

collect_stats_per_getsize

collect_stats_per_putthruput

collect_stats_per_numthreads


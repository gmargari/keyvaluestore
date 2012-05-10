#!/bin/bash

executable="/tmp/build/src/sim"
kvstorefolder="/tmp/kvstore/"

#============================
# my_print()
#============================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

#========================================================
# mailme()
#========================================================
mailme() {
    echo $* | mail -s "message from `hostname`" gmargari@gmail.com
}

#========================================================
# put_only_sim()
#========================================================
put_only_sim() {
    command="${executable} -m ${memsize} -i ${datasize} -c ${method}"
    my_print $command

    rm -rf ${kvstorefolder}/*
    mkdir -p ${statsfolder}
    $command > ${statsfolder}/${prefix}${suffix}.stats 2> /dev/null && return
    
    echo "Error running $command"
    mailme "Error running $command"
    exit 1
}

#========================================================
# put_get_sim()
#========================================================
put_get_sim() {
    command="${executable} -m ${memsize} -i ${datasize} -c ${method}"
    command=$command" -P ${putthrput} -G ${getthrput} -g ${getthreads} -R ${getsize} -t"
    my_print $command

    rm -rf ${kvstorefolder}/*
    mkdir -p ${statsfolder}
    $command > ${statsfolder}/${prefix}${suffix}.stats 2> ${statsfolder}/${prefix}${suffix}.stderr && return
    
    echo "Error running $command"
    mailme "Error running $command"
    exit 1
}

#========================================================
# Put_only_experiments()
#========================================================
Put_only_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize
   
    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        method=${methods[$i]}
        prefix=${prefixes[$i]}
        suffix=""
        put_only_sim
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Memsize_experiments()
#========================================================
Memsize_experiments() {
    my_print "${FUNCNAME}()"

    datasize=$def_datasize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        method=${methods[$i]}
        prefix=${prefixes[$i]}
        for memsize in ${memsizes[@]}; do
            suffix="-memsize-$memsize"
            put_only_sim
        done
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Put_get_experiments()
#========================================================
Put_get_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize
    
    putthrput=$def_putthrput
    getthrput=$def_getthrput
    getthreads=$def_getthreads
    getsize=$def_getsize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        method=${methods[$i]}
        prefix=${prefixes[$i]}
        suffix="-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}"
        put_get_sim
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Putthrput_experiments()
#========================================================
Putthrput_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize
    
    putthrput=$def_putthrput
    getthrput=$def_getthrput
    getthreads=$def_getthreads
    getsize=$def_getsize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        method=${methods[$i]}
        prefix=${prefixes[$i]}
        for putthrput in ${putthrputs[@]}; do
            suffix="-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}"
            put_get_sim
        done
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Getsize_experiments()
#========================================================
Getsize_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize
    
    putthrput=$def_putthrput
    getthrput=$def_getthrput
    getthreads=$def_getthreads
    getsize=$def_getsize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        method=${methods[$i]}
        prefix=${prefixes[$i]}
        for getsize in ${getsizes[@]}; do
            suffix="-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}"
            put_get_sim
        done
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Numgetthreads_experiments()
#========================================================
Numgetthreads_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize
    
    putthrput=$def_putthrput
    getthrput=$def_getthrput
    getthreads=$def_getthreads
    getsize=$def_getsize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        method=${methods[$i]}
        prefix=${prefixes[$i]}
        for getthreads in ${numgetthreads[@]}; do
            getthrput=$(( $def_getthrput / $getthreads ))
            suffix="-pthrput-${putthrput}-gthrput-${getthrput}-gthreads-${getthreads}-gsize-${getsize}"
            put_get_sim
        done
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Zipfkeys_experiments()
#========================================================
Zipfkeys_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        prefix=${prefixes[$i]}
        if [ "${methods[$i]}" == "rangemerge" ]; then
            for zipf_a in ${rngmerge_zipf_as[@]}; do
                method="${methods[$i]} -z ${zipf_a} -u"
                suffix="-zipf_a-${zipf_a}"
                put_only_sim
            done
        else
            for zipf_a in ${zipf_as[@]}; do
                method="${methods[$i]} -z ${zipf_a} -u"
                suffix="-zipf_a-${zipf_a}"
                put_only_sim
            done
        fi
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Orderedkeys_experiments()
#========================================================
Orderedkeys_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize

    if [ ${#methods[*]} -ne ${#prefixes[*]} ]; then
        echo "Error: different array size of 'methods' and 'prefixes'"
        exit 1
    fi

    for (( i=0; i < ${#methods[*]}; i++ )); do
        prefix=${prefixes[$i]}
        if [ "${methods[$i]}" == "rangemerge" ]; then
            for ordered_prob in ${rngmerge_ordered_probs[@]}; do
                method="${methods[$i]} -o ${ordered_prob}"
                suffix="-ord-prob-${ordered_prob}"
                put_only_sim
            done
        else
            for ordered_prob in ${ordered_probs[@]}; do
                method="${methods[$i]} -o ${ordered_prob}"
                suffix="-ord-prob-${ordered_prob}"
                put_only_sim
            done
        fi
    done

    mailme "${FUNCNAME}()"
}

#========================================================
# Rangemerge_blocksize()
#========================================================
Rangemerge_blocksize() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize

    prefix="rangemerge"
    for blocksize in ${blocksizes[@]}; do
        method="rangemerge -b ${blocksize}"
        suffix="-blocksize-${blocksize}"
        put_only_sim
    done

    mailme "${FUNCNAME}()"
}






#===================================================================================
# main script starts here
#===================================================================================

# check user gave results folder as argument
if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi     
statsfolder=$1

rm -rf ${kvstorefolder}/*
rm -rf ${statsfolder}/*

if [ ! -f ${executable} ]; then
    echo "Error: ${executable} not found"
    exit 1
fi

# assert that debug level is 0 before starting experiments
if [ "`${executable} -c rangemerge -i 0 2> /dev/null | grep debug_level | awk '{print $3}'`" != "0" ]; then 
    echo "Error: debug level not zero"
    exit 1
fi

#---------------------------------
# You can change these
#---------------------------------
def_datasize=10100
def_memsize=512

def_putthrput=2500
def_getthreads=1
def_getthrput=20
def_getsize=10

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -r 4" "geometric -p 2" "geometric -p 3" "geometric -p 4"
           "cassandra -l 2" "cassandra -l 3" "cassandra -l 4" "rangemerge" "immediate" )
prefixes=( "nomerge" "geometric-r-2" "geometric-r-3" "geometric-r-4" "geometric-p-2" "geometric-p-3" "geometric-p-4"
           "cassandra-l-2" "cassandra-l-3" "cassandra-l-4" "rangemerge" "immediate" )

Put_only_experiments

Put_get_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "cassandra -l 2" "cassandra -l 4" "rangemerge" "immediate" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "cassandra-l-2"  "cassandra-l-4"  "rangemerge" "immediate" )

## memsizes=( "0128" "0256" "0512" "1024" "2048" )
memsizes=(    "0128" "0256"        "1024" "2048" )
Memsize_experiments

methods=(  "geometric -r 2" "geometric -r 3" "geometric -p 2" "cassandra -l 4" "rangemerge" "immediate" )
prefixes=( "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "cassandra-l-4"  "rangemerge" "immediate" )

## putthrputs=( "1000" "2500" "5000" "10000" "20000" "40000" )
putthrputs=(    "1000"        "5000" "10000" "20000" "40000" )
Putthrput_experiments

## getsizes=( "1" "10" "100" "1000" "10000" "100000" )
getsizes=(    "1"      "100" "1000" "10000" "100000" )
Getsize_experiments

## numgetthreads=( "1" "2" "5" "10" "20" )
numgetthreads=(        "2" "5" "10" "20" )
Numgetthreads_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "cassandra -l 2" "cassandra -l 4" "rangemerge" "immediate" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "cassandra-l-2"  "cassandra-l-4"  "rangemerge" "immediate" )

zipf_as=( "0.2" "1.0" "2.0" "3.0" "4.0" )
rngmerge_zipf_as=( "0.2" "0.5" "1.0" "1.5" "2.0" "2.5" "3.0" "3.5" "4.0" )
Zipfkeys_experiments

ordered_probs=( "0.0" "1.0" )
rngmerge_ordered_probs=( "0.0" "0.2" "0.4" "0.6" "0.8" "1.0" )
Orderedkeys_experiments

blocksizes=( "0000" "0016" "0032" "0064" "0128" "0256" "0512" "1024" "2048" "4096" )
Rangemerge_blocksize


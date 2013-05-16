#!/bin/bash

executable="/tmp/build/src/sim"

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
# async_start_disk_stats()
#========================================================
async_start_disk_stats() {

    if [ $# -ne 1 ]; then
        echo "${FUNCNAME}(): log file argument missing"
        exit 1
    fi 
    logfile=$1

    iostat -k -d -x 5 > $logfile &
}

#========================================================
# async_start_cpu_stats()
#========================================================
async_start_cpu_stats() {

    if [ $# -ne 1 ]; then
        echo "${FUNCNAME}(): log file argument missing"
        exit 1
    fi 
    logfile=$1

    mpstat 5 > $logfile &
}

#========================================================
# stop_disk_stats()
#========================================================
stop_disk_stats() {
    if [ $# -ne 1 ]; then
        echo "${FUNCNAME}(): log file argument missing"
        exit 1
    fi 
    logfile=$1

    pkill iostat
    tmpfile=$(mktemp)
    iostat -k -d -x 1 1 | grep Device > $tmpfile
    cat $logfile | grep "^sda "    >> $tmpfile
    mv $tmpfile $logfile
}

#========================================================
# stop_cpu_stats()
#========================================================
stop_cpu_stats() {
    if [ $# -ne 1 ]; then
        echo "${FUNCNAME}(): log file argument missing"
        exit 1
    fi 
    pkill mpstat
}

#========================================================
# put_only_sim()
#========================================================
put_only_sim() {
    command="${executable} -m ${memsize} -i ${datasize} -c ${method}"
    my_print $command

    rm -rf ${kvstorefolder}/*
    mkdir -p ${statsfolder}
    filename=${statsfolder}/${prefix}${suffix}
    
    async_start_disk_stats ${filename}.iostat
    async_start_cpu_stats ${filename}.mpstat
    $command > ${filename}.stats 2> /dev/null && 
    stop_disk_stats ${filename}.iostat &&
    stop_cpu_stats ${filename}.mpstat &&
    return
    
    echo "Error running ${command}"
    mailme "Error running ${command}"
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
    filename=${statsfolder}/${prefix}${suffix}

    async_start_disk_stats ${filename}.iostat
    async_start_cpu_stats ${filename}.mpstat
    $command > ${filename}.stats 2> ${filename}.stderr && 
    stop_disk_stats ${filename}.iostat && 
    stop_cpu_stats ${filename}.mpstat && 
    return
    
    echo "Error running ${command}"
    mailme "Error running ${command}"
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
# Putthrput_experiments()
#========================================================
Putthrput_experiments() {
    my_print "${FUNCNAME}()"

    memsize=$def_memsize
    datasize=$def_datasize
    
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
# Getthrput_experiments()
#========================================================
Getthrput_experiments() {
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
        for getthrput in ${getthrputs[@]}; do
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
            if [ "$getthreads" == "0" ]; then
                getthrput=$def_getthrput
            else
                getthrput=$(( $def_getthrput / $getthreads ))
            fi
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
# Rangemerge_blocksize_experiments()
#========================================================
Rangemerge_blocksize_experiments() {
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

# check if executable exists
if [ ! -f ${executable} ]; then
    echo "Error: ${executable} not found"
    exit 1
fi

# check if sample run executes without problems
cmd="${executable} -c nomerge -i 0"
${cmd} 2> /dev/null > /dev/null
retval=$?
if [ $retval -ne 0 ]; then
    echo "Error running: ${cmd}"
    ${cmd}
    exit 1
fi

# check if index folder exists
kvstorefolder=`${executable} -c nomerge -i 0 2> /dev/null | grep 'index_dir:' | awk '{print $3}'`
echo $kvstorefolder
if [ ! -d ${kvstorefolder} ]; then
    echo "Error: folder ${kvstorefolder} does not exist"
    exit 1
fi

# ensure that debug level is set to 0 before starting experiments
if [ "`${executable} -c nomerge -i 0 2> /dev/null | grep 'debug_level:' | awk '{print $3}'`" != "0" ]; then
    echo "Error: debug level not zero"
    exit 1
fi

echo "index folder: ${kvstorefolder}"
echo "stats folder: ${statsfolder}"

#---------------------------------
# You can change these
#---------------------------------
def_datasize=10100
def_memsize=512

def_putthrput=2500
def_getthreads=1
def_getthrput=20
def_getsize=10

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
Put_only_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
putthrputs=( "1000" "2500" "5000" "10000" "20000" "40000" "0" )   
Putthrput_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
## getthrputs=( "1" "2" "5" "10" "20" "40" "0" )
getthrputs=(            "5" "10"      "40" "0" )
Getthrput_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
## getsizes=( "1" "10" "100" "1000" "10000" "100000" )
getsizes=(    "1"      "100" "1000" "10000" "100000" )
Getsize_experiments  

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
## numgetthreads=( "1" "2" "5" "10" "20" )
numgetthreads=(        "2" "5" "10" "20" )
Numgetthreads_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
## memsizes=( "0128" "0256" "0512" "1024" "2048" )
memsizes=(    "0128" "0256"        "1024" "2048" )
Memsize_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
## zipf_as=( "0.0" "1.0" "2.0" "3.0" "4.0" )
zipf_as=(    "0.0"       "2.0"       "4.0" )
rngmerge_zipf_as=( "0.0" "0.5" "1.0" "1.5" "2.0" "2.5" "3.0" "3.5" "4.0" )
Zipfkeys_experiments

methods=(  "nomerge" "geometric -r 2" "geometric -r 3" "geometric -p 2" "sma -l 4" "rangemerge" "remerge" )
prefixes=( "nomerge" "geometric-r-2"  "geometric-r-3"  "geometric-p-2"  "sma-l-4"  "rangemerge" "remerge" )
ordered_probs=( "0.00" "0.50" "1.00" )
rngmerge_ordered_probs=( "0.00" "0.25" "0.50" "0.75" "1.00" )
Orderedkeys_experiments

## blocksizes=( "0032" "0064" "0128" "0256" "0512" "1024" "2048" "0000" )
blocksizes=(    "0032" "0064" "0128"        "0512" "1024" "2048" "0000" )
Rangemerge_blocksize_experiments

methods=(  "rangemerge -b 0032" "rangemerge -b 0064" "rangemerge -b 0128" "rangemerge -b 0512" "rangemerge -b 1024" "rangemerge -b 2048" "rangemerge -b 0000" )
prefixes=( "rangemerge-b-0032"  "rangemerge-b-0064"  "rangemerge-b-0128"  "rangemerge-b-0512"  "rangemerge-b-1024"  "rangemerge-b-2048"  "rangemerge-b-0000"  )
putthrputs=( "2500" )   
Putthrput_experiments

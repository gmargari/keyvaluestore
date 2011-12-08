#!/bin/bash

executable="/home/gmargari/Dropbox/phd/keyvaluestore/build/src/sim"
kvstorefolder="/tmp/kvstore/"

#========================================================
# execute_sim()
#========================================================
execute_sim() {
    command="${executable} -m ${memsize} -i ${bytesins} -c ${mergername} ${mergerparams} ${extraparams}"
    echo $command
    rm -rf ${kvstorefolder}/*
    mkdir -p ${statsfolder}
    $command > ${statsfolder}/${outputname}.stats && return
    
    echo "Error running $command"
    echo "Error running $command" | mail -s "Error running kvstore experiment" -t "gmargari@gmail.com"
    exit 1
}

#========================================================
# all_methods()
#========================================================
all_methods() {
    memsize=$def_memsize_in_mb
    bytesins=$def_mb_to_ins
    mergername="nomerge";      mergerparams="";        outputname="nomerge";            execute_sim;
    mergername="geometric";    mergerparams="-r 2";    outputname="geometric-r-2";      execute_sim;
    mergername="geometric";    mergerparams="-r 3";    outputname="geometric-r-3";      execute_sim;
    mergername="geometric";    mergerparams="-r 4";    outputname="geometric-r-4";      execute_sim;
    mergername="geometric";    mergerparams="-p 4";    outputname="geometric-p-4";      execute_sim;
    mergername="geometric";    mergerparams="-p 3";    outputname="geometric-p-3";      execute_sim;
    mergername="geometric";    mergerparams="-p 2";    outputname="geometric-p-2";      execute_sim;
    mergername="rangemerge";   mergerparams="";        outputname="rangemerge";         execute_sim;
    mergername="immediate";    mergerparams="";        outputname="immediate";          execute_sim;
    mergername="cassandra";    mergerparams="-l 2";    outputname="cassandra-l-2";      execute_sim;
    mergername="cassandra";    mergerparams="-l 3";    outputname="cassandra-l-3";      execute_sim;
    mergername="cassandra";    mergerparams="-l 4";    outputname="cassandra-l-4";      execute_sim;
#    mergername="logarithmic";  mergerparams="";        outputname="logarithmic";        execute_sim; # identical to Geometric r = 2
#    mergername="geometric";    mergerparams="-p 1";    outputname="geometric-p-1";      execute_sim; # identical to Immediate Merge
#    mergername="rangemerge";   mergerparams="-b 0";    outputname="rangemerge-blocksize-000";  execute_sim; # identical to Immediate Merge
}

#========================================================
# rangemerge_memsize()
#========================================================
rangemerge_memsize() {
    bytesins=$def_mb_to_ins
    mergername="rangemerge"
    for memsize in ${memsizes[@]}; do
        mergerparams="-b 256 -f 0";  outputname="rangemerge-memsize-$memsize";  execute_sim;
    done
}

#========================================================
# nomerge_memsize()
#========================================================
nomerge_memsize() {
    bytesins=$def_mb_to_ins
    mergername="nomerge"
    for memsize in ${memsizes[@]}; do
        mergerparams="";  outputname="nomerge-memsize-$memsize";  execute_sim;
    done
}

#========================================================
# immediate_memsize()
#========================================================
immediate_memsize() {
    bytesins=$def_mb_to_ins
    mergername="immediate"
    for memsize in ${memsizes[@]}; do
        mergerparams="";  outputname="immediate-memsize-$memsize";  execute_sim;
    done
}

#========================================================
# geometric_r_memsize()
#========================================================
geometric_r_memsize() {
    bytesins=$def_mb_to_ins
    mergername="geometric"
    for r in 2 3 4; do
        for memsize in ${memsizes[@]}; do
            mergerparams="-r $r";  outputname="geometric-r-${r}-memsize-${memsize}";  execute_sim;
        done
    done
}

#========================================================
# geometric_p_memsize()
#========================================================
geometric_p_memsize() {
    bytesins=$def_mb_to_ins
    mergername="geometric"
    for p in 2 3 4; do
        for memsize in ${memsizes[@]}; do
            mergerparams="-p $p";  outputname="geometric-p-${p}-memsize-${memsize}";  execute_sim;
        done
    done
}

#========================================================
# cassandra_memsize()
#========================================================
cassandra_memsize() {
    bytesins=$def_mb_to_ins
    mergername="cassandra"
    for l in 2 3 4; do
        for memsize in ${memsizes[@]}; do
            mergerparams="-l $l";  outputname="cassandra-l-${l}-memsize-${memsize}";  execute_sim;
        done
    done
}

#========================================================
# rangemerge_flushmem()
#========================================================
rangemerge_flushmem() {
    memsize=$def_memsize_in_mb
    bytesins=$def_mb_to_ins   
    mergername="rangemerge"
    for fmem in ${flushmemsizes[@]}; do
        mergerparams="-b 256 -f $fmem";  outputname="rangemerge-flushmem-$fmem";  execute_sim;
    done
}

#========================================================
# rangemerge_blocksize()
#========================================================
rangemerge_blocksize() {
    memsize=$def_memsize_in_mb
    bytesins=$def_mb_to_ins
    mergername="rangemerge"
    for bsize in ${blocksizes[@]}; do
        mergerparams="-b $bsize -f 0";  outputname="rangemerge-blocksize-$bsize";  execute_sim;
    done
}



#========================================================
# main script starts here
#========================================================

# check user gave results folder as argument
if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi     
statsfolder=$1

rm -rf ${kvstorefolder}/*
rm -rf ${statsfolder}/*

# assert that debug level is 0 before starting experiments
if [ "`${executable} -c rangemerge -i 0 | grep debug_level | awk '{print $3}'`" != "0" ]; then 
    echo "Error: debug level not zero"
    exit 1
fi

#---------------------------------
# You can change these
#---------------------------------
#extraparams="-z -u" # -u to make keys unique, else more than 90% of keys will be replaced in memory and thus not inserted!
extraparams=""
blocksizes=( "0000" "0016" "0032" "0064" "0128" "0256" "0512" "1024" "2048" "4096" ) # for rangemerge
flushmemsizes=( "0000" "0004" "0008" "0016" "0032" "0064" "0128" "0256" "0512" "1024" ) # for rangemerge
memsizes=( "0128" "0256" "0512" "1024" "2048" )
## def_mb_to_ins=5000 # dont use odd number of GB (e.g. for memsize=2GB, inserting 5GB will result 4GB written (2 flushes of 2GB, 1Gb left in memory))
#def_mb_to_ins=10150 # a bit more MB in order to totally fill memstore
def_mb_to_ins=20250 # a bit more MB in order to totally fill memstore
def_memsize_in_mb=1024
#---------------------------------
# You can change these
#---------------------------------

echo -e "\e[1;31m [ $0: all methods ] \e[m"
all_methods

echo -e "\e[1;31m [ $0: nomerge memsize ] \e[m"
nomerge_memsize

echo -e "\e[1;31m [ $0: geometric-r memsize ] \e[m"
geometric_r_memsize

echo -e "\e[1;31m [ $0: geometric-p memsize ] \e[m"
geometric_p_memsize

echo -e "\e[1;31m [ $0: rangemerge memsize ] \e[m"
rangemerge_memsize

echo -e "\e[1;31m [ $0: immediate memsize ] \e[m"
immediate_memsize

echo -e "\e[1;31m [ $0: cassandra memsize ] \e[m"
cassandra_memsize

echo -e "\e[1;31m [ $0: rangemerge blocksize ] \e[m"
rangemerge_blocksize

echo -e "\e[1;31m [ $0: rangemerge flushmem ] \e[m"
rangemerge_flushmem

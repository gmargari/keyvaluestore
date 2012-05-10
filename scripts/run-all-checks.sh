#!/bin/bash

tmpfile="/tmp/deletethis.123"

function run_check()
{
    script=$1

    rm -rf /tmp/kvstore/*
    echo -e "\n\n\e[1;32m===== Running $script =====\e[m\n\n"

    rm -f $tmpfile
    
    $script || touch $tmpfile
    
    if [ -f $tmpfile ]; then
    	echo -e "Error running \e[1;31m${script}\e[m"
    	rm $tmpfile
    	exit 1 
    fi
    
}

mkdir -p /tmp/kvstore/

run_check /tmp/build/src/test/vfiletest
#run_check /tmp/build/src/test/kvstoretest
run_check /tmp/build/src/test/maptest
run_check /tmp/build/src/test/memstoretest
run_check /tmp/build/src/test/vfileindextest
# check-kvstore does not stop
#./scripts/check-kvstore.sh && 
run_check ./scripts/check-save-and-load-kvstore.sh
run_check ./scripts/check-kvstore.sh
run_check ./scripts/check-md5sums-identical.sh
run_check ./scripts/check-cppstyle.sh
run_check ./scripts/check-memleaks.sh

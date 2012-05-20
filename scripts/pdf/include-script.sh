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

#============================
# check_file_exist()
#============================
check_file_exist() {
    local file
    for file in "$@"; do
        if [ ! -f "$file" ]; then
            echo "$file does not exist!" >&2
        fi
    done
}

#============================
# ensure_file_exist()
#============================
ensure_file_exist() {
    local file
    for file in "$@"; do
        if [ ! -f "$file" ]; then
            echo "$file does not exist!" >&2
            exit 1
        fi
    done
}

#========================================================
# average_using_sliding_window()
#========================================================
windowsize=15
function average_using_sliding_window {
    local tmp="$(mktemp)"
    cat $1 | awk -v w=$windowsize \
      '{sumt += $2 - t[w]; suml += $3 - l[w]; \
        for (i = w; i >= 2; i--) {t[i] = t[i-1]; l[i] = l[i-1];} t[1] = $2; l[1] = $3; 
        if (NR >= w) print $1, (sumt>0 ? sumt/w : 0), (suml>0 ? suml/w : 0)}' > $tmp
    mv $tmp $1
}

#========================================================
# keep_get_stats()
#========================================================
function keep_get_stats {
    # get_stats rows: "[GET_STATS] <timestamp> <time_elapsed> <num_requests> <totallatency>
    cat $1 | awk '{if ($1=="[GET_STATS]") {print $2, ($3?$4/$3:0), ($4?$5/$4:0)}}'
}

#========================================================
# keep_flush_stats()
#========================================================
function keep_flush_stats {
    # get_stats rows: "[GET_STATS] <timestamp> <time_elapsed> <num_requests> <totallatency>
    cat $1 | awk '{if ($1=="[FLUSH_MEM]") {print $2, $3, $4}}'
}


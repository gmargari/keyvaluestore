memsize="1"
bytestoinsert="10"
generalparams="-g 1"
leakfile=/tmp/delete.memcheck
simlogfile=/tmp/delete.simlog

execute() {
    method=$1
    params=$2
    
    cmd="./build/src/sim -m $memsize -i $bytestoinsert $generalparams -c $method"
    echo $cmd | awk '{printf "%-60s ", $0}'
            
    rm -rf /tmp/kvstore/* &&
#    valgrind --tool=memcheck --leak-check=full $cmd 2>&1 | grep "^=="| awk '$0~"blocks$"{print $0}'
    valgrind --tool=memcheck --show-reachable=yes --leak-check=full --log-file=$leakfile $cmd 2>&1 > $simlogfile
    leaked=`cat $leakfile | grep "in use at exit" | awk '{print $6}'`
    if [ "$leaked" != "28" ]; then
        echo -e "\n\e[1;31m ===== bytes leaked: $leaked ===== \e[m"
        echo -e "\n\e[1;31m ===== program log: $simlogfile ===== \e[m\n"
        cat $simlogfile
        echo -e "\n\e[1;31m ===== valgrind log: $leakfile ===== \e[m\n"
        cat $leakfile
        exit 1
    else
        echo "no leaks"
    fi
}

mkdir -p /tmp/kvstore

execute "nomerge"
execute "logarithmic" 
execute "geometric  -r 2"
execute "geometric  -r 3"
execute "geometric  -r 4"
execute "geometric  -p 4"
execute "geometric  -p 3"
execute "geometric  -p 2"
execute "rangemerge -b 2"
execute "immediate"
execute "cassandra  -l 2"
execute "cassandra  -l 4"
execute "cassandra  -l 10"

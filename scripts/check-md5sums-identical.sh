memsize="1"
bytestoinsert="10"
generalparams="-g 3"
prevmd5sum=""
md5file="/tmp/delete.md5sum"


execute() {
    method=$1
    params=$2
    
    cmd="./build/src/sim -m $memsize -i $bytestoinsert $generalparams -c $method"
    echo $cmd | awk '{printf "%-60s ", $0}'
    
    rm -rf /tmp/kvstore/* &&
    $cmd > /dev/null &&
    ./build/src/tools/merge `cat /tmp/kvstore/dstore.info | grep "^/tmp/kvstore/dfile"` | grep -v ^"#" | awk '{print $1 $2 $3}' |  md5sum > $md5file &&
    md5sum=`cat $md5file | awk '{print $1}'`
    echo $md5sum
    if [ -n "$prevmd5sum"  -a  "$prevmd5sum" != "$md5sum" ]; then
        echo "md5sums differ!"
        exit 1
    fi
    prevmd5sum=$md5sum
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

rm $md5file

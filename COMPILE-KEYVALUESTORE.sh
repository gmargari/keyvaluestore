mkdir -p build &&
cd build &&
cmake .. &&
make &&

#echo "Executable in build/bin"
#cmake --graphviz=/tmp/a.dot .. &&
#dot -Tps /tmp/a.dot -o graphviz_call_graph.ps

true
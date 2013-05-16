BUILDDIR=/tmp/build/
CURDIR=`pwd`
mkdir -p ${BUILDDIR} &&
cd ${BUILDDIR} &&
cmake ${CURDIR} &&
make &&

echo "Executable: ${BUILDDIR}src/sim"
#cmake --graphviz=/tmp/a.dot .. &&
#dot -Tps /tmp/a.dot -o graphviz_call_graph.ps

true
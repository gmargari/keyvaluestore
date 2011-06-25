cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

add_executable (kvtcheck kvtcheck.cpp)
add_executable (kvtcat kvtcat.cpp)
add_executable (kvtmerge kvtmerge.cpp)
target_link_libraries (kvtcat KVTDiskFile KVTDiskFileInputStream)
target_link_libraries (kvtcheck KVTDiskFile KVTDiskFileInputStream)
target_link_libraries (kvtmerge KVTDiskFile KVTDiskFileInputStream KVTPriorityInputStream)

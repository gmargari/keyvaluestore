cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

add_executable (kvcheck kvcheck.cpp)
add_executable (kvcat kvcat.cpp)
target_link_libraries (kvcat KVDiskFile KVDiskFileInputStream)
target_link_libraries (kvcheck KVDiskFile KVDiskFileInputStream)

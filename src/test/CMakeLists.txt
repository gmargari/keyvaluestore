cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

add_executable (kvtstoretest kvtstoretest.cpp)
target_link_libraries (kvtstoretest KeyValueStore)

add_executable (kvtmaptest kvtmaptest.cpp)
target_link_libraries (kvtmaptest KeyValueStore KVTMapInputStream KVTDiskFileInputStream KVTDiskFileOutputStream KVTPriorityInputStream)

add_executable (vfileindextest vfileindextest.cpp)
target_link_libraries (vfileindextest KeyValueStore VFileIndex)
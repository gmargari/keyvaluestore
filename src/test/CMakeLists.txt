cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

add_executable (kvstoretest kvstoretest.cpp)
target_link_libraries (kvstoretest KeyValueStore Scanner)

add_executable (maptest maptest.cpp)
target_link_libraries (maptest KeyValueStore MapInputStream DiskFileInputStream DiskFileOutputStream PriorityInputStream)

add_executable (vfileindextest vfileindextest.cpp)
target_link_libraries (vfileindextest KeyValueStore VFileIndex)

add_executable (vfiletest vfiletest.cpp)
target_link_libraries (vfiletest KeyValueStore VFile)

add_executable (requestthrottletest requestthrottletest.cpp)

add_executable (memstoretest memstoretest.cpp)
target_link_libraries (memstoretest MemStore)

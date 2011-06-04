cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

add_executable (kvstoretest kvstoretest.cpp)
target_link_libraries (kvstoretest KeyValueStore)

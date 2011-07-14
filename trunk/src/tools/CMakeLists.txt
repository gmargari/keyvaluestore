cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

add_executable (check check.cpp)
add_executable (cat cat.cpp)
add_executable (merge merge.cpp)
target_link_libraries (cat DiskFile DiskFileInputStream)
target_link_libraries (check DiskFile DiskFileInputStream)
target_link_libraries (merge DiskFile DiskFileInputStream PriorityInputStream)

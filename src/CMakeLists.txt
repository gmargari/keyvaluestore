cmake_minimum_required (VERSION 2.6)
project (KEYVALUESTORE)

# add_subdirectory (test)

add_library (KeyValueStore              KeyValueStore.cpp)
add_library (MemStore                   MemStore.cpp)
add_library (DiskStore                  DiskStore.cpp)
add_library (KVMap                      KVMap.cpp)
add_library (KVMapInputStream           KVMapInputStream.cpp)
add_library (KVDiskFile                 KVDiskFile.cpp)
add_library (KVDiskFileInputStream      KVDiskFileInputStream.cpp)
add_library (CompactionManager          CompactionManager.cpp)
add_library (KVSerialization            KVSerialization.cpp)
add_library (VFile                      VFile.cpp)
add_library (Simulator                  Simulator.cpp)

target_link_libraries (MemStore         KVMap)
target_link_libraries (DiskStore        KVDiskFile VFile)
target_link_libraries (KVDiskFile       KVSerialization)
target_link_libraries (VFile            Simulator )

# target_link_libraries (KVDiskFileScanner KVScanner)
# target_link_libraries (KVMapScanner KVScanner)

target_link_libraries (CompactionManager MemStore DiskStore KVMapInputStream KVDiskFileInputStream )

target_link_libraries (KeyValueStore MemStore DiskStore CompactionManager)

add_executable (kvstore main.cpp)
target_link_libraries (kvstore KeyValueStore)

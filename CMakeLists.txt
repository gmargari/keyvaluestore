cmake_minimum_required (VERSION 2.6)

project (KEYVALUESTORE)
# -Wno-long-long: stop complaining about use of long long constants
set (CMAKE_CXX_FLAGS "-g -Wall -Wundef -ansi -pedantic -D_FILE_OFFSET_BITS=64 -Weffc++ -Wno-long-long -Wshadow")
#set (CMAKE_CXX_FLAGS "-g -Wall -Wundef -ansi -pedantic -D_FILE_OFFSET_BITS=64 -Wno-long-long")
#set (CMAKE_CXX_FLAGS "-g -Wall -Wundef -ansi -D_FILE_OFFSET_BITS=64")
add_subdirectory (src)

#set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#message ( "+--------------------------------------------------" )
#message ( "| Executable path --> " ${EXECUTABLE_OUTPUT_PATH} )
#message ( "+--------------------------------------------------" )

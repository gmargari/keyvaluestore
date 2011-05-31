cmake_minimum_required (VERSION 2.6)

project (KEYVALUESTORE)
set (CMAKE_CXX_FLAGS "-g -Wall -Wundef -ansi -pedantic -D_FILE_OFFSET_BITS=64")
#set (CMAKE_CXX_FLAGS "-g -Wall -Wundef -ansi -D_FILE_OFFSET_BITS=64")
add_subdirectory (src)

#set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 
#message ( "+--------------------------------------------------" )
#message ( "| Executable path --> " ${EXECUTABLE_OUTPUT_PATH} )
#message ( "+--------------------------------------------------" )

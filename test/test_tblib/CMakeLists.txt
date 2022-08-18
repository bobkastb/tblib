cmake_minimum_required (VERSION 3.8)

project(test_tblib)	

add_definitions( -fpermissive -std=c++17 -Wno-multichar )

file(GLOB TBLIB_TEST   *.cpp)
add_executable (test_tblib ${TBLIB_TEST} )

#include_directories( ../.. )	

target_link_libraries(test_tblib  tblib dl pthread)	

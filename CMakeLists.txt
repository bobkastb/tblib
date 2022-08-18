# CMakeList.txt : CMake project for prj_1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project(tblib)	
# Add source to this project's executable.

add_definitions( -fpermissive -std=c++17 -Wno-multichar )

include_directories( . )	
set(TBLIB_MAIN tb_basetypes.h tb_exception_base.h )	
#set(TBLIB_GDATA gdata/t_string.cpp )	#gdata/tbt_locale.cpp)	
#set(TBLIB_GDATA gdata/t_str_utils.cpp )
file(GLOB TBLIB_GDATA   gdata/*.cpp  gdata/tests/*.cpp   gdata/hashes/*.cpp   gdata/pack/*.cpp	gdata/cipher/*.cpp )
file(GLOB TBLIB_OTHER   conf/*.cpp   parse/*.cpp   com/*.cpp)
file(GLOB TBLIB_TSYS   tsys/*.cpp)
#dependences
#file(GLOB TBLIB_TSYS   tsys/*.h)
#file(GLOB TBLIB_OTHER   conf/*.h   parse/*.h   com/*.h)

#file(GLOB TBLIB_GDATA gdata/tests/*.cpp )

add_library(tblib STATIC ${TBLIB_MAIN} ${TBLIB_GDATA} ${TBLIB_OTHER} ${TBLIB_TSYS} )

add_subdirectory ("test/test_tblib")

# TODO: Add tests and install targets if needed.

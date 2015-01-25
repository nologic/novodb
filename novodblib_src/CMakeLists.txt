# Build Novodblib
FILE(GLOB NDB_LIB_CPP Independent/*.cpp 
                      Independent/modules/*.cpp
                      ${CMAKE_SYSTEM_NAME}/*.cpp)

INCLUDE_DIRECTORIES(../libs/netlib_include 
                    ../libs/yara_include
					../libs/lldb_include
					Independent)


ADD_LIBRARY(novolib STATIC ${NDB_LIB_CPP})

message(STATUS "Novodb library files: ${NDB_LIB_CPP}")

install(TARGETS novolib DESTINATION ../Build)
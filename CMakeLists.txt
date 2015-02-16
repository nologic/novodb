cmake_minimum_required (VERSION 2.8)
project (Novodb)

message(STATUS OS:${CMAKE_SYSTEM_NAME})
message(STATUS PROCESSOR:${CMAKE_SYSTEM_PROCESSOR})

IF(CMAKE_SYSTEM_NAME EQUAL "Darwin")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11 -stdlib=libc++")
ELSEIF(CMAKE_SYSTEM_NAME EQUAL "Linux")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DLINUX -std=c++11")
ENDIF()

add_subdirectory(novodblib_src)
add_subdirectory(novodbweb_src)

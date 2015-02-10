cmake_minimum_required (VERSION 2.8)
project (Novodb)

message(STATUS OS:${CMAKE_SYSTEM_NAME})
message(STATUS PROCESSOR:${CMAKE_SYSTEM_PROCESSOR})

IF(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11 -stdlib=libc++")
ELSE(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DLINUX -std=c++11")
ENDIF (APPLE)

add_subdirectory(novodblib_src)
add_subdirectory(novodbweb_src)

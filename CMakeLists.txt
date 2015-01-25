cmake_minimum_required (VERSION 2.8)
project (Novodb)

message(STATUS OS:${CMAKE_SYSTEM_NAME})
message(STATUS PROCESSOR:${CMAKE_SYSTEM_PROCESSOR})

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++11 -stdlib=libc++")

add_subdirectory(novodblib_src)
add_subdirectory(novodbweb_src)
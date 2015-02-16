# Build Novodblib
# order of the libraries matters very much. If a symbol is not requested by the time
# it is loaded by linker then it is thrown out.
FILE(GLOB NDB_WEB_CPP Independent/*.cpp 
                      ${CMAKE_SYSTEM_NAME}/*.cpp)

set(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost COMPONENTS regex filesystem log system thread REQUIRED)
message(STATUS ${Boost_LIBRARIES})
message(STATUS ${Boost_INCLUDE_DIR})

INCLUDE_DIRECTORIES(../libs/civetweb_include
					../novodblib_src/Independent
					Independent
					${Boost_INCLUDE_DIR})

ADD_EXECUTABLE(novodbweb ${NDB_WEB_CPP})

TARGET_LINK_LIBRARIES(novodbweb novolib)

FIND_LIBRARY(NETLIB cppnetlib-uri PATHS ../libs NO_DEFAULT_PATH)
FIND_LIBRARY(CIVETLIB NAMES civetweb PATHS ../libs NO_DEFAULT_PATH)
FIND_LIBRARY(YARA_LIBRARY NAMES yara PATHS ../libs NO_DEFAULT_PATH)

TARGET_LINK_LIBRARIES(novodbweb ${CIVETLIB})
TARGET_LINK_LIBRARIES(novodbweb ${EXTRA_LIBS} )
TARGET_LINK_LIBRARIES(novodbweb ${YARA_LIBRARY} )
TARGET_LINK_LIBRARIES(novodbweb ${NETLIB} )
TARGET_LINK_LIBRARIES(novodbweb ${Boost_LIBRARIES})

IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# find apple frameworks.
	FIND_LIBRARY(LLDB_LIBRARY LLDB PATHS ../libs)
   	MARK_AS_ADVANCED (LLDB_LIBRARY)
  	SET(EXTRA_LIBS ${LLDB_LIBRARY})

	FIND_LIBRARY(SYSTEM_LIBRARY NAMES System PATHS ../libs)
	TARGET_LINK_LIBRARIES(novodbweb ${SYSTEM_LIBRARY})

	FIND_LIBRARY(CRYPTO_LIBRARY NAMES crypto PATHS ../libs)
	TARGET_LINK_LIBRARIES(novodbweb ${CRYPTO_LIBRARY})
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	FIND_LIBRARY(LLDB_LIBRARY lldb PATHS /usr/lib/llvm-3.4/lib)
	FIND_LIBRARY(M_LIBRARY m)
	FIND_LIBRARY(DL_LIBRARY dl)
	
	TARGET_LINK_LIBRARIES(novodbweb ${LLDB_LIBRARY})
	TARGET_LINK_LIBRARIES(novodbweb ${M_LIBRARY})
	TARGET_LINK_LIBRARIES(novodbweb ${DL_LIBRARY})
ENDIF ()

message(STATUS "Novodb Webserver files: ${NDB_WEB_CPP}")

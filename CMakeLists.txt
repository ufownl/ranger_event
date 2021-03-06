CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(EVENT)

# silence policy CMP0042 warning by enabling RPATH explicitly
IF(APPLE AND NOT DEFINED CMAKE_MACOSX_RPATH)
	SET(CMAKE_MACOSX_RPATH TRUE)
ENDIF()

SET(CMAKE_MODULE_PATH ${EVENT_SOURCE_DIR}/cmake)
SET(BUILD_SHARED_LIBS ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FIND_PACKAGE(Libevent2 REQUIRED)

ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(sample)

FIND_PACKAGE(SWIG)
FIND_PACKAGE(Lua)
IF(SWIG_FOUND AND LUA_FOUND)
	ADD_SUBDIRECTORY(lua_api)
ENDIF()

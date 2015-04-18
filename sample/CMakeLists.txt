SET(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -std=c++11")

INCLUDE_DIRECTORIES(${EVENT_SOURCE_DIR})

ADD_EXECUTABLE(simple_client simple_client.cpp)
ADD_EXECUTABLE(rate_limit rate_limit.cpp)
ADD_EXECUTABLE(echo_server echo_server.cpp)

ADD_DEPENDENCIES(simple_client ranger_event)
ADD_DEPENDENCIES(rate_limit ranger_event)
ADD_DEPENDENCIES(echo_server ranger_event)

TARGET_LINK_LIBRARIES(simple_client ranger_event)
TARGET_LINK_LIBRARIES(rate_limit ranger_event)
TARGET_LINK_LIBRARIES(echo_server ranger_event)

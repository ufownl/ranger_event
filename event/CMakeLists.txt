SET(EVENT_HEADER
		buffer.hpp
		dispatcher.hpp
		endpoint.hpp
		signal.hpp
		token_bucket_cfg.hpp
		tcp_acceptor.hpp
		tcp_connection.hpp
		timer.hpp
		trigger.hpp)
SET(EVENT_SOURCE
		buffer.cpp
		dispatcher.cpp
		endpoint.cpp
		signal.cpp
		token_bucket_cfg.cpp
		tcp_acceptor.cpp
		tcp_connection.cpp
		timer.cpp
		trigger.cpp)

SET(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -std=c++11 -DRANGER_EVENT_INTERNAL")

ADD_LIBRARY(ranger_event
		${EVENT_HEADER}
		${EVENT_SOURCE})
TARGET_LINK_LIBRARIES(ranger_event event_core event_pthreads)

INSTALL(TARGETS ranger_event DESTINATION lib)
INSTALL(FILES ${EVENT_HEADER} DESTINATION include/event)

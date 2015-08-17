FIND_PATH(Libevent2_INCLUDE_DIR
	NAMES
		event2/buffer.h
		event2/bufferevent.h
		event2/event.h
		event2/listener.h
		event2/thread.h
	PATHS
		/opt/include
		/usr/include
		/usr/local/include
)

FIND_LIBRARY(Libevent2_LIBRARY_CORE
	NAMES
		event_core
	PATHS
		/opt/lib
		/usr/lib
		/usr/local/lib
)

FIND_LIBRARY(Libevent2_LIBRARY_PTHREADS
	NAMES
		event_pthreads
	PATHS
		/opt/lib
		/usr/lib
		/usr/local/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libevent2
	FOUND_VAR
		Libevent2_FOUND
	REQUIRED_VARS
		Libevent2_INCLUDE_DIR
		Libevent2_LIBRARY_CORE
		Libevent2_LIBRARY_PTHREADS
)

SET(Libevent2_LIBRARIES ${Libevent2_LIBRARY_CORE} ${Libevent2_LIBRARY_PTHREADS})

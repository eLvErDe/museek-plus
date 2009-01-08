project(NewNet CXX)

# Find Event
find_package(Event REQUIRED)
include_directories(${Event_INCLUDE_DIRS})

if (Event_LIBRARIES AND EVENT_FOUND)
    set(NEWNET_SOURCES
      nnbuffer.cpp
      nnclientsocket.cpp
      nnlog.cpp
      nnpath.cpp
      nnratelimiter.cpp
      nntcpserversocket.cpp
      nnreactor.cpp
      nnserversocket.cpp
      nntcpclientsocket.cpp
    )

    if(UNIX)
      set(NEWNET_SOURCES
        ${NEWNET_SOURCES}
        nnunixclientsocket.cpp
        nnunixserversocket.cpp
      )
    endif(UNIX)

    add_library(NewNet STATIC ${NEWNET_SOURCES})

    # Link the newnet binary to some libraries.
    target_link_libraries(
      NewNet
      ${Event_LIBRARIES}
    )
else(Event_LIBRARIES AND EVENT_FOUND)
    MESSAGE("!!! NewNet will NOT be installed..")
endif(Event_LIBRARIES AND EVENT_FOUND)

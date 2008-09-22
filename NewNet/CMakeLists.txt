project(NewNet CXX)
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

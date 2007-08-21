project(Museekal CXX)

set(MUSEEKAL_SOURCES
  ClientConnection.cc
  ConnectionManager.cc 
  ListenConnection.cc
  TrafficTracker.cc
)


add_library(Museekal STATIC ${MUSEEKAL_SOURCES})

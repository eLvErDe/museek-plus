project(Museek CXX)

include_directories(${TOOLS_SOURCE_DIR})

set(MUSEEK_SOURCES
BaseConnection.cc
DistribConnection.cc
DistribManager.cc
MessageConnection.cc
Messages.cc
Museek.cc
PeerConnection.cc
PeerListener.cc
PeerManager.cc
Recoder.cc
ServerConnection.cc
ShareBase.cc
TransferConnection.cc
TransferManager.cc
)


add_library(Museek STATIC ${MUSEEK_SOURCES})

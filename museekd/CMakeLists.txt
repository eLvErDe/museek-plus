project(museekd CXX)

# Find ZLib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

# Find LibXml2
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
add_definitions(${LIBXML2_DEFINITIONS})

# Check for some OS specific libraries
set(OS_LIBRARIES "")
if(WIN32 AND NOT CYGWIN)
  set(OS_LIBRARIES ${OS_LIBRARIES} ole32 shell32)
endif(WIN32 AND NOT CYGWIN)

if (NOT MANDIR)
  set(MANUAL_DIRECTORY "man/man1")
else (NOT MANDIR)
  set(MANUAL_DIRECTORY ${MANDIR}/man1)
endif (NOT MANDIR)

# Select some source files.
set(MUSEEKD_SOURCES
  codesetmanager.cpp  ifacemanager.cpp     peermanager.cpp
  configmanager.cpp   ifacesocket.cpp      peersocket.cpp
  main.cpp             servermanager.cpp
  downloadmanager.cpp messageprocessor.cpp sharesdatabase.cpp
  downloadsocket.cpp  museekd.cpp          ticketsocket.cpp
  handshakesocket.cpp networkmessage.cpp   usersocket.cpp
  uploadmanager.cpp   uploadsocket.cpp     searchmanager.cpp
  distributedsocket.cpp
)

# Build the museekd binary.
add_executable(museekd ${MUSEEKD_SOURCES})

# Link the museekd binary to some libraries.
target_link_libraries(
  museekd
  Mucipher
  Muhelp
  ${NEWNET_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${ICONV_LIBRARIES}
  ${OS_LIBRARIES}
)

# Install the museekd binary to the 'bin' directory.
install(
  TARGETS museekd
  DESTINATION bin
)

install(
  FILES "museekd.1"
  DESTINATION ${MANUAL_DIRECTORY}
)
install(
  FILES "config.xml.tmpl"
  DESTINATION ${DATADIR}/museek/museekd
)
MESSAGE("--> Museek Daemon (museekd) will be installed..")

project(museekd CXX)

# Find ZLib
find_package(ZLIB REQUIRED)
# Find LibXml2
find_package(LibXml2 REQUIRED)
# Find libxml++
find_package(libxml++ REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})
add_definitions(${LIBXML2_DEFINITIONS})

include_directories(${LIBXMLPP_INCLUDE_DIR})
ADD_DEFINITIONS(${LIBXMLPP_CFLAGS})

include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${TOOLS_SOURCE_DIR})
include_directories(${MUSEEK_SOURCE_DIR})
include_directories(${MUSEEKAL_SOURCE_DIR})
include_directories(${MUHELP_SOURCE_DIR})
include_directories(${MUCIPHER_SOURCE_DIR})

# Check for some OS specific libraries
set(OS_LIBRARIES "")

# Select some source files.
set(MUSEEKD_SOURCES
   Buddies.cc IfaceConnection.cc main.cc museekd.cc
)

# Build the museekd binary.
add_executable(museekd ${MUSEEKD_SOURCES})

# Link the museekd binary to some libraries.
target_link_libraries(
  museekd

  Museek
  Museekal
  Muhelp
  Mucipher
  ${ZLIB_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${LIBXMLPP_LIBRARY}
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
MESSAGE("--> Museek Daemon (museekd) will be installed..")

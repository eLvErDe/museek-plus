project(Muhelp CXX)

INCLUDE(FindPkgConfig REQUIRED)

pkg_search_module(LIBXMLPP "libxml++-2.6")

ADD_DEFINITIONS(${LIBXMLPP_CFLAGS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BUILD_DIR})

set(MUHELP_SOURCES
Codec.cc
DirEntry.cc
Muconf.cc
)

add_library(Muhelp STATIC ${MUHELP_SOURCES})

# Link the Muhelp library to some libraries.
target_link_libraries(
  Muhelp
  NewNet
  ${LIBXML2_LIBRARIES}
  ${LIBXMLPP_LIBRARIES}
)

project(Muhelp CXX)

include(FindPkgConfig REQUIRED)

pkg_search_module(LIBXMLPP REQUIRED "libxml++-2.6")

add_definitions(${LIBXMLPP_CFLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_CURRENT_BUILD_DIR})

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

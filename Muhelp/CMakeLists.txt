project(Muhelp CXX)

find_package(libxml++ REQUIRED)
ADD_DEFINITIONS(${LIBXMLPP_CFLAGS})

set(MUHELP_SOURCES
Codec.cc
DirEntry.cc
Muconf.cc
Mulog.cc
)


add_library(Muhelp STATIC ${MUHELP_SOURCES})

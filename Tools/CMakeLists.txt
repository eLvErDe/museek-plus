project(Tools CXX)

find_package(libxml++ REQUIRED)
ADD_DEFINITIONS(${LIBXMLPP_CFLAGS})

set(TOOLS_SOURCES
  mp3.c
  scandir.cc
  scanner.cc
)
set(MUSCAN_SOURCES
  muscan.cc
  ${TOOLS_SOURCES}
) 
set(MUSCAND_SOURCES
  muscand.cc
  ${TOOLS_SOURCES}
)
add_library(Tools STATIC ${TOOLS_SOURCES})

add_executable(muscan ${MUSCAN_SOURCES})

# To build, or not build muscand
if (FAM_LIBRARIES AND FAM_H)

add_executable(muscand ${MUSCAND_SOURCES})

target_link_libraries(
  muscand
  Muhelp
  ${ZLIB_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${FAM_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${LIBXMLPP_LIBRARY}
  ${ICONV_LIBRARIES}
  ${OS_LIBRARIES}
)

install(
  TARGETS muscand
  DESTINATION bin
)
endif(FAM_LIBRARIES AND FAM_H)

target_link_libraries(
  muscan
  Muhelp
  ${ZLIB_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${LIBXMLPP_LIBRARY}
  ${ICONV_LIBRARIES}
  ${OS_LIBRARIES}
)


# Install the museekd binary to the 'bin' directory.
install(
  TARGETS muscan
  DESTINATION bin
)

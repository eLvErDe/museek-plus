project(Tools CXX)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE(PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(PREFIX)

find_package(FAM)

INCLUDE(FindPkgConfig REQUIRED)
pkg_search_module(LIBXMLPP "libxml++-2.6")
ADD_DEFINITIONS(${LIBXMLPP_CFLAGS})

set(TOOLS_SOURCES
  mp3.c
  scandir.cc
  scanner.cc
)

add_library(Tools STATIC ${TOOLS_SOURCES})

if (C_MUSCAN)

set(MUSCAN_SOURCES
  muscan.cc
  ${TOOLS_SOURCES}
) 
set(MUSCAND_SOURCES
  muscand.cc
  ${TOOLS_SOURCES}
)

add_executable(muscan ${MUSCAN_SOURCES})

# Install the muscan binary to the 'bin' directory.
install(
  TARGETS muscan
  DESTINATION bin
)


set(MANPAGES
  muscan.1
)
MESSAGE("--> muscan (file scanner) will be installed..")
# To build, or not build muscand
if (FAM_LIBRARIES AND FAM_FOUND)
    add_executable(muscand ${MUSCAND_SOURCES})

    target_link_libraries(
      muscand
      Muhelp
      ${ZLIB_LIBRARIES}
      ${FAM_LIBRARIES}
      ${VORBIS_LIBRARIES}
      ${ICONV_LIBRARIES}
      ${OS_LIBRARIES}
    )
    set(MANPAGES
      ${MANPAGES}
      muscand.1
    )

    install(
      TARGETS muscand
      DESTINATION bin
    )
    MESSAGE("--> muscand (FAM-based file scanning daemon) will be installed..")
else(FAM_LIBRARIES AND FAM_FOUND)
    MESSAGE("!!! muscand will NOT be installed..")
endif(FAM_LIBRARIES AND FAM_FOUND)

target_link_libraries(
  muscan
  Muhelp
  ${ZLIB_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${ICONV_LIBRARIES}
  ${OS_LIBRARIES}
)


install(
  FILES ${MANPAGES}
  DESTINATION ${MANUAL_DIRECTORY}
)
endif  (C_MUSCAN)

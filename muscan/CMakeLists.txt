project(Tools CXX)

if(PREFIX)
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
    set(CMAKE_INSTALL_PREFIX "/usr")
endif()

find_package(FAM)

include(FindPkgConfig REQUIRED)
pkg_search_module(LIBXMLPP "libxml++-2.6")
add_definitions(${LIBXMLPP_CFLAGS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(TOOLS_SOURCES
    mp3.c
    scandir.cc
    scanner.cc
    )

add_library(Tools STATIC ${TOOLS_SOURCES})

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
message("--> muscan (file scanner) will be installed.")
# To build, or not build muscand
if(FAM_LIBRARIES AND FAM_FOUND)
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
    message("--> muscand (FAM-based file scanning daemon) will be installed.")
else()
    message("!!! muscand will NOT be installed.")
endif()

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

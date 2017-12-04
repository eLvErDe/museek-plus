project(Murmur)

set(MURMUR_SOURCES
    murmur
    )

project(Pymurmur)

file(GLOB_RECURSE PYMURMUR_FILES pymurmur/*.py)

set(PYMURMUR_SOURCES
    ${PYMURMUR_FILES}
    )

EXECUTE_PROCESS(COMMAND python2 ${CMAKE_SOURCE_DIR}/cmake/get_python_lib.py museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if(NOT sitepackages OR error)
    message(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif()

install(
    FILES ${PYMURMUR_SOURCES}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}/pymurmur
    )


# Install the murmur binary to the 'bin' directory.
install(
    FILES ${MURMUR_SOURCES}
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
set(MANPAGES
    murmur.1
    )

install(
    FILES ${MANPAGES}
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    )
set(ICONS
    images/murmur-96px.png
    images/murmur-64px.png
    images/murmur-32px.png
    images/murmur-16px.png
    )

install(
    FILES ${ICONS}
    DESTINATION "share/pixmaps"
    )
set(DESKTOP
    files/murmur.desktop
    )

install(
    FILES ${DESKTOP}
    DESTINATION "share/applications"
    )
message("--> Murmur will be installed.")

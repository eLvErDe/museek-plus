project(Mucous)

# add_subdirectory(pymucous)
set(MUCOUS_SOURCES
    mucous
    )

project(Pymucous)

file(GLOB_RECURSE PYMUCOUS_FILES pymucous/*.py)

set(PYMUCOUS_SOURCES
    ${PYMUCOUS_FILES}
    )

execute_process(COMMAND python2
    ${CMAKE_SOURCE_DIR}/cmake/get_python_lib.py museek
    OUTPUT_VARIABLE sitepackages
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE error)

if(NOT sitepackages OR error)
    message(FATAL_ERROR
        "Failed to find Python's site-packages directory"
        ${sitepackages} ${error})
endif()

install(
    FILES ${PYMUCOUS_SOURCES}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}/pymucous
    )


# Install the muscan binary to the 'bin' directory.
install(
    FILES ${MUCOUS_SOURCES}
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

set(MANPAGES
    mucous.1
    )

install(
    FILES ${MANPAGES}
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    )
message("--> Mucous will be installed.")

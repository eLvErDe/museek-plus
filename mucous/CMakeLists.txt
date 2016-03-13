project(Mucous)

if(PREFIX)
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
    set(CMAKE_INSTALL_PREFIX "/usr")
endif()


if(NOT MANDIR)
    set(MANUAL_DIRECTORY "man/man1")
else()
    set(MANUAL_DIRECTORY ${MANDIR}/man1)
endif()

# add_subdirectory(pymucous)
set(MUCOUS_SOURCES
    mucous
    )

project(Pymucous)

file(GLOB_RECURSE PYMUCOUS_FILES pymucous/*.py)

set(PYMUCOUS_SOURCES
    ${PYMUCOUS_FILES}
    )

execute_process(COMMAND python
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
    DESTINATION ${MANUAL_DIRECTORY}
    )
message("--> Mucous will be installed..")

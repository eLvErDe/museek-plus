project(Clients)

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

set(CLIENT_SOURCES
    mulog
    museekchat
    musirc.py
    museekcontrol
    )

# Install the scripts to the 'bin' directory.
install(
    FILES ${CLIENT_SOURCES}
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

set(MANPAGES
    mulog.1
    museekcontrol.1
    )

install(
    FILES ${MANPAGES}
    DESTINATION ${MANUAL_DIRECTORY}
    )

message("--> mulog, museekchat, museekcontrol, musirc.py will be installed..")

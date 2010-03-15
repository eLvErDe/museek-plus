project(SetupTools)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE(PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(PREFIX)

if (NOT MANDIR)
  set(MANUAL_DIRECTORY "man/man1")
else (NOT MANDIR)
  set(MANUAL_DIRECTORY ${MANDIR}/man1)
endif (NOT MANDIR)

set(SETUP_TOOLS_SOURCES
  musetup
  musetup-gtk
  musetup-qt
)

# Install the musetup executables to the 'bin' directory.
install(
  FILES ${SETUP_TOOLS_SOURCES}
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

set(MANPAGES
  musetup.1
  musetup-gtk.1  
  musetup-qt.1  
)

set(ICONS
  museek_alt2.png
)

install(
  FILES ${ICONS}
  DESTINATION "share/pixmaps"
)

install(
  FILES ${MANPAGES}
  DESTINATION ${MANUAL_DIRECTORY}
)

set(DESKTOP
  musetup-gtk.desktop
)
install(
  FILES ${DESKTOP}
  DESTINATION "share/applications"
)
message("--> musetup, musetup-gtk, musetup-qt will be installed..")

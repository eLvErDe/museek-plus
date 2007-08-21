project(SetupTools)

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
)


install(
  FILES ${MANPAGES}
  DESTINATION ${MANUAL_DIRECTORY}
)

set(ICONS
  museek_alt2.png
)

install(
  FILES ${ICONS}
  DESTINATION "share/pixmaps"
)
set(DESKTOP
  musetup-gtk.desktop
)
install(
  FILES ${DESKTOP}
  DESTINATION "share/applications"
)
message("--> musetup, musetup-gtk, musetup-qt will be installed..")

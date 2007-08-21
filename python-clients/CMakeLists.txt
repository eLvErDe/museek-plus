project(Clients)

set(CLIENT_SOURCES
  mulog
  museekchat
  musirc.py
  museekcontrol
)

# Install the muscan binary to the 'bin' directory.
install(
  FILES ${CLIENT_SOURCES}
  DESTINATION bin
  PERMISSIONS WORLD_EXECUTE
)

set(MANPAGES
  mulog.1
  museekcontrol.1  
)


install(
  FILES ${MANPAGES}
  DESTINATION "man/man1"
)
message("--> mulog, museekchat, museekcontrol, musirc.py will be installed..")

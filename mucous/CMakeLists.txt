project(Mucous)


# add_subdirectory(pymucous)
set(MUCOUS_SOURCES
  mucous
)

project(Pymucous)

file(GLOB_RECURSE PYMUCOUS_FILES pymucous *.py)

set(PYMUCOUS_SOURCES
  ${PYMUCOUS_FILES}
)
EXECUTE_PROCESS(COMMAND python ../python-site-packages.py museek OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)
if (NOT version OR error)
	EXECUTE_PROCESS(COMMAND python python-site-packages.py museek OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE)
endif (NOT version OR error)
install(
  FILES ${PYMUCOUS_SOURCES}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${version}
)


# Install the muscan binary to the 'bin' directory.
install(
  FILES ${MUCOUS_SOURCES}
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

set(MANPAGES
  mucous.1
)

install(
  FILES ${MANPAGES}
  DESTINATION ${MANUAL_DIRECTORY}
)
MESSAGE("--> Mucous will be installed..")

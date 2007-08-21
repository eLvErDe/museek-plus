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
EXECUTE_PROCESS(COMMAND python ../python-site-packages.py pymucous OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE)

install(
  FILES ${PYMUCOUS_SOURCES}
  DESTINATION ${PREFIX}/${version}
)


# Install the muscan binary to the 'bin' directory.
install(
  FILES ${MUCOUS_SOURCES}
  DESTINATION bin
  PERMISSIONS WORLD_EXECUTE
)

set(MANPAGES
  mucous.1
)

install(
  FILES ${MANPAGES}
  DESTINATION "man/man1"
)
MESSAGE("--> Mucous will be installed..")
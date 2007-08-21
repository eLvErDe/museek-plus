project(Murmur)

set(MURMUR_SOURCES
  murmur
)

project(Pymurmur)

file(GLOB_RECURSE PYMURMUR_FILES pymurmur *.py)

set(PYMURMUR_SOURCES
  ${PYMURMUR_FILES}
)
EXECUTE_PROCESS(COMMAND python ../python-site-packages.py pymurmur OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE)

install(
  FILES ${PYMURMUR_SOURCES}
  DESTINATION ${PREFIX}/${version}
)


# Install the murmur binary to the 'bin' directory.
install(
  FILES ${MURMUR_SOURCES}
  DESTINATION bin
  PERMISSIONS WORLD_EXECUTE
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
MESSAGE("--> Murmur will be installed..")
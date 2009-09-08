project(Murmur)

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

set(MURMUR_SOURCES
  murmur
)

project(Pymurmur)

file(GLOB_RECURSE PYMURMUR_FILES pymurmur/*.py)

set(PYMURMUR_SOURCES
  ${PYMURMUR_FILES}
)

EXECUTE_PROCESS(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1,0,'')" museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if (NOT sitepackages OR error)
	MESSAGE(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif (NOT sitepackages OR error)

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
  DESTINATION ${MANUAL_DIRECTORY}
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

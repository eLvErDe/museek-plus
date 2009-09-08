project(Mucous)


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

# add_subdirectory(pymucous)
set(MUCOUS_SOURCES
  mucous
)


project(Pymucous)

file(GLOB_RECURSE PYMUCOUS_FILES pymucous/*.py)

set(PYMUCOUS_SOURCES
  ${PYMUCOUS_FILES}
)

EXECUTE_PROCESS(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1,0,'')" museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if (NOT sitepackages OR error)
	MESSAGE(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif (NOT sitepackages OR error)

install(
  FILES ${PYMUCOUS_SOURCES}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}/pymucous
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

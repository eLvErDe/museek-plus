project(Museek-Python-Bindings)

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

set(PYTHON_BINDINGS_SOURCES
museek/__init__.py
museek/messages.py
museek/driver.py
)

EXECUTE_PROCESS(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1,0,'')" museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if (NOT sitepackages OR error)
	MESSAGE(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif (NOT sitepackages OR error)

install(
  FILES ${PYTHON_BINDINGS_SOURCES}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}/museek
)
message("--> Python Bindings will be installed..")

# This example shows how to use python
# Currently these languages have been tested:
#  perl tcl ruby php4 pike
project(PyMucipher)


FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(mucipherc.py GENERATED)
SWIG_ADD_MODULE(mucipherc python aes.c hexdigest.c md5.c sha.c sha256.c wraphelp.c  mucipher.i )
SWIG_LINK_LIBRARIES(mucipherc ${PYTHON_LIBRARIES})

set(PYMUCIPHER_LIBS
  mucipher.py
  ${CMAKE_CURRENT_BINARY_DIR}/mucipherc.py
  ${CMAKE_CURRENT_BINARY_DIR}/_mucipherc.so
)

EXECUTE_PROCESS(COMMAND python ../python-site-packages.py  OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)
if (NOT version OR error)
	EXECUTE_PROCESS(COMMAND python python-site-packages.py  OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE)
endif (NOT version OR error)
install(
  FILES ${PYMUCIPHER_LIBS}
  DESTINATION ${PREFIX}/${version}
)
MESSAGE("--> PyMucipher will be installed.. [NO_PYMUCIPHER=1]")
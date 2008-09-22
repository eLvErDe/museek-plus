# This example shows how to use python
# Currently these languages have been tested:
#  perl tcl ruby php4 pike
project(PyMucipher)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE(PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(PREFIX)

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(mucipherc.py GENERATED)
SET_SOURCE_FILES_PROPERTIES(mucipher.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(mucipher.i PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(mucipherc python ${Mucipher_SOURCE_DIR}/aes.c ${Mucipher_SOURCE_DIR}/hexdigest.c ${Mucipher_SOURCE_DIR}/md5.c ${Mucipher_SOURCE_DIR}/sha.c ${Mucipher_SOURCE_DIR}/sha256.c ${Mucipher_SOURCE_DIR}/wraphelp.c mucipher.i )
SWIG_LINK_LIBRARIES(mucipherc ${PYTHON_LIBRARIES})

set(PYMUCIPHER_LIBS
  mucipher.py
  ${CMAKE_CURRENT_BINARY_DIR}/mucipherc.py
  ${CMAKE_CURRENT_BINARY_DIR}/_mucipherc.so
)
EXECUTE_PROCESS(COMMAND python -c "import sys; import os; print os.path.join('lib', 'python' + str(sys.version_info[0]) + '.' + str(sys.version_info[1]), 'site-packages')" museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if (NOT sitepackages OR error)
	MESSAGE(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif (NOT sitepackages OR error)

install(
  FILES ${PYMUCIPHER_LIBS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}
)
MESSAGE("--> PyMucipher will be installed.. [NO_PYMUCIPHER=1]")

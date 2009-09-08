# This example shows how to use python
# Currently these languages have been tested:
#  perl tcl ruby php4 pike
project(PyMucipher)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE(PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(PREFIX)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

FIND_PACKAGE(SWIG REQUIRED)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(mucipherc.py GENERATED)
SET_SOURCE_FILES_PROPERTIES(mucipher.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(mucipher.i PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(mucipherc python ${Mucipher_SOURCE_DIR}/aes.cpp ${Mucipher_SOURCE_DIR}/hexdigest.cpp ${Mucipher_SOURCE_DIR}/md5.cpp ${Mucipher_SOURCE_DIR}/sha.cpp ${Mucipher_SOURCE_DIR}/sha256.cpp ${Mucipher_SOURCE_DIR}/wraphelp.cpp mucipher.i )
SWIG_LINK_LIBRARIES(mucipherc ${PYTHON_LIBRARIES})

set(PYMUCIPHER_LIBS
  mucipher.py
  ${CMAKE_CURRENT_BINARY_DIR}/mucipherc.py
  ${CMAKE_CURRENT_BINARY_DIR}/_mucipherc.so
)
EXECUTE_PROCESS(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1,0,'')" museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if (NOT sitepackages OR error)
	MESSAGE(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif (NOT sitepackages OR error)

install(
  FILES ${PYMUCIPHER_LIBS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}
)
MESSAGE("--> PyMucipher will be installed.. [NO_PYMUCIPHER=1]")

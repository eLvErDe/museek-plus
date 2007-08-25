project(Museek-Python-Bindings)


set(PYTHON_BINDINGS_SOURCES
museek/__init__.py
museek/messages.py
museek/driver.py
)

EXECUTE_PROCESS(COMMAND python -c "import sys; import os; print os.path.join('lib', 'python' + str(sys.version_info[0]) + '.' + str(sys.version_info[1]), 'site-packages')" museek OUTPUT_VARIABLE sitepackages OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)

if (NOT sitepackages OR error)
	MESSAGE(FATAL_ERROR "Failed to find Python's site-packages directory" ${sitepackages} ${error})
endif (NOT sitepackages OR error)

install(
  FILES ${PYTHON_BINDINGS_SOURCES}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}/museek
)
message("--> Python Bindings will be installed..")

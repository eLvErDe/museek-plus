project(Museek-Python-Bindings)


set(PYTHON_BINDINGS_SOURCES
museek/__init__.py
museek/messages.py
museek/driver.py
)
EXECUTE_PROCESS(COMMAND python ../python-site-packages.py museek OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_VARIABLE error)
if (NOT version OR error)
	EXECUTE_PROCESS(COMMAND python python-site-packages.py museek OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE)
endif (NOT version OR error)

install(
  FILES ${PYTHON_BINDINGS_SOURCES}
  DESTINATION ${PREFIX}/${version}
)
message("--> Python Bindings will be installed..")

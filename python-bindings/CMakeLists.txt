project(Museek-Python-Bindings)

set(PYTHON_BINDINGS_SOURCES
    museek/__init__.py
    museek/messages.py
    museek/driver.py
    )

execute_process(COMMAND python2
    ${CMAKE_SOURCE_DIR}/cmake/get_python_lib.py museek
    OUTPUT_VARIABLE sitepackages
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE error)

if(NOT sitepackages OR error)
    message(FATAL_ERROR
        "Failed to find Python's site-packages directory"
        ${sitepackages} ${error})
endif()

install(
    FILES ${PYTHON_BINDINGS_SOURCES}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${sitepackages}/museek
    )
message("--> Python Bindings will be installed.")

project(Mucipher CXX)
set(MUCIPHER_SOURCES
  aes.cpp
  hexdigest.cpp
  md5.cpp
  sha.cpp
  sha256.cpp
)
add_library(Mucipher STATIC ${MUCIPHER_SOURCES})

if (PYMUCIPHER)
  add_subdirectory(PyMucipher)
else (PYMUCIPHER)
  MESSAGE("!!! PyMucipher will NOT be installed.. [NO_PYMUCIPHER=0]")
endif (PYMUCIPHER)


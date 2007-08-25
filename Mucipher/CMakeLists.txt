project(Mucipher C)
set(MUCIPHER_SOURCES
  aes.c
  hexdigest.c
  md5.c
  sha.c
  sha256.c
)
include_directories(..)
add_library(Mucipher STATIC ${MUCIPHER_SOURCES})

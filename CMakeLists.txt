project(museekd)

# Include some cmake checks.
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckCXXSourceCompiles)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF(PREFIX)

# # Enable NewNet's smart-pointer debugging capabilities.
# add_definitions(-DNN_PTR_DEBUG -DNN_PTR_DEBUG_ASSERT)

# Add top source directory to the include path.
include_directories(.)

# Check for common include files.
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(stdio.h HAVE_STDIO_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stddef.h HAVE_STDDEF_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(inttypes.h, HAVE_INTTYPES_H)
check_include_files(time.h HAVE_TIME_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
check_include_files(sys/epoll.h HAVE_EPOLL_CTL)
check_include_files(sys/signal.h HAVE_SIGNAL_H)
check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(sys/syslog.h HAVE_SYSLOG_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_files(sys/dir.h HAVE_SYS_DIR_H)
check_include_files(ndir.h HAVE_NDIR_H)
check_include_files(vorbis/vorbisfile.h HAVE_VORBIS_H)
check_include_files(pwd.h HAVE_PWD_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
check_include_files(netinet/tcp.h HAVE_NETINET_TCP_H)
check_include_files(windows.h HAVE_WINDOWS_H)
check_include_files(winsock.h HAVE_WINSOCK_H)

# Check for some standard types and type sizes
check_type_size("unsigned char" UNSIGNED_CHAR)
check_type_size("unsigned short" UNSIGNED_SHORT)
check_type_size("unsigned int" UNSIGNED_INT)
check_type_size("unsigned long" UNSIGNED_LONG)
check_type_size("unsigned long long" UNSIGNED_LONG_LONG)
check_type_size("uint" UINT)
check_type_size("uint32" UINT32)

# Determine the type to use as uint32.
if(HAVE_UINT32)
  set(UINT32_TYPE uint32)
elseif(UNSIGNED_CHAR EQUAL 4)
  set(UINT32_TYPE "unsigned char")
elseif(UNSIGNED_SHORT EQUAL 4)
  set(UINT32_TYPE "unsigned short")
elseif(UNSIGNED_INT EQUAL 4)
  set(UINT32_TYPE "unsigned int")
elseif(UNSIGNED_LONG EQUAL 4)
  set(UINT32_TYPE "unsigned long")
elseif(UNSIGNED_LONG_LONG EQUAL 4)
  set(UINT32_TYPE "unsigned long long")
else(UNSIGNED_LONG_LONG EQUAL 4)
  message(FATAL_ERROR "Couldn't determine 32bit unsigned interger type.")
endif(HAVE_UINT32)

# Check for iconv.h
check_include_files(iconv.h HAVE_ICONV_H)
if(NOT HAVE_ICONV_H)
  MESSAGE(FATAL_ERROR "Could not find iconv.h.")
endif(NOT HAVE_ICONV_H)

# Check for iconv library (if we need it)
check_function_exists(iconv HAVE_ICONV_BUILTIN)
if(HAVE_ICONV_BUILTIN)
  set(ICONV_LIBRARIES "")
else(HAVE_ICONV_BUILTIN)
  find_library(ICONV_LIBRARIES NAMES iconv PATHS /usr/lib /usr/local/lib)
  if(NOT ICONV_LIBRARIES)
    message(FATAL_ERROR "Couldn't find iconv library.")
  endif(NOT ICONV_LIBRARIES)
endif(HAVE_ICONV_BUILTIN)

# Use the iconv library (if any) for the following tests.
set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})

# Check if iconv() takes const char **.
check_cxx_source_compiles("
  #include <iconv.h>
  int main(int argc, char ** argv)
  { return iconv(0, (const char **)0, 0, (char**)0, 0); }
" ICONV_CONST)

# Check if iconv() takes char **.
check_cxx_source_compiles("
  #include <iconv.h>
  int main(int argc, char ** argv)
  { return iconv(0, (char **)0, 0, (char**)0, 0); }
" ICONV_NON_CONST)

# Not const char ** AND not char **? Uhh.. That's bad mmkay.
if(NOT ICONV_CONST AND NOT ICONV_NON_CONST)
  message(FATAL_ERROR "Problem with iconv: could not determine iconv() input type.")
endif(NOT ICONV_CONST AND NOT ICONV_NON_CONST)

# Check if signal type.
check_cxx_source_compiles("
  #include <signal.h>
  void test_handler(int s) { };
  int main() {
     signal(11, test_handler); }
" SIGNAL_TYPE)
	
if (NOT SIGNAL_TYPE)
 set(RSIGTYPE 1)
endif (NOT SIGNAL_TYPE)

# Clean up after iconv tests.
set(CMAKE_REQUIRED_LIBRARIES)

# Check for FAM
check_include_files(fam.h FAM_H)
if(NOT FAM_H)
  MESSAGE(STATUS "Could not find fam.h.")
endif(NOT FAM_H)

find_library(FAM_LIBRARIES NAMES fam PATHS /usr/lib /usr/local/lib)
if(NOT FAM_LIBRARIES)
  message(STATUS "Couldn't find fam library.")
endif(NOT FAM_LIBRARIES)

if (FAM_LIBRARIES AND FAM_H)
  message(STATUS "FAM found at ${FAM_LIBRARIES}. muscand will be compiled")
  set(HAVE_FAM_H 1)
else (FAM_LIBRARIES AND FAM_H)
  message(STATUS "FAM not supported. muscand will not be compiled")
  set(HAVE_FAM_H 0)
endif (FAM_LIBRARIES AND FAM_H)

set(INCLUDE_HEAD "")
if (HAVE_DIRENT_H)
   set(INCLUDE_HEAD  "${INCLUDE_HEAD}#include <dirent.h>\n")
endif (HAVE_DIRENT_H)

if (HAVE_SYS_DIR_H)
   set(INCLUDE_HEAD "${INCLUDE_HEAD}#include <sys/dir.h>\n")
endif (HAVE_SYS_DIR_H)

if (HAVE_SYS_NDIR_H)
   set(INCLUDE_HEAD "${INCLUDE_HEAD}#include <sys/ndir.h>\n")
endif (HAVE_SYS_NDIR_H)

if (HAVE_NDIR_H)
   set(INCLUDE_HEAD "${INCLUDE_HEAD}#include <ndir.h>\n")
endif (HAVE_NDIR_H)

set(code "
  ${INCLUDE_HEAD}
  int main() {
    struct dirent **temp;
    scandir(0, &temp, 0, 0);
    return 0; }
" )
check_cxx_source_compiles("${code}" DIR_H)
if (NOT DIR_H)
  message(FATAL_ERROR "Headers for DIRENT not found")
endif (NOT DIR_H)

# Detect Vorbisfile library
if (HAVE_VORBIS_H)
  find_library(VORBIS_LIBRARIES NAMES vorbisfile PATHS /usr/lib /usr/local/lib)
  if (VORBIS_LIBRARIES)
    set(HAVE_VORBIS 1)
    message(STATUS "Vorbisfile library found at ${VORBIS_LIBRARIES}.")
  else (VORBIS_LIBRARIES)
    message(STATUS "Vorbisfile library wasn't found, ogg metadata will not be created")
    set(VORBIS_LIBRARIES "")
  endif (VORBIS_LIBRARIES)
else (HAVE_VORBIS_H)
  set(VORBIS_LIBRARIES "")
endif (HAVE_VORBIS_H)

set(CMAKE_REQUIRED_LIBRARIES)

if (NOT HAVE_STDLIB_H)
  message(FATAL_ERROR "STDLIB not found")
endif (NOT HAVE_STDLIB_H)
# Process system.h.cmake to system.h.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/system.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/system.h)
# Add -DHAVE_CONFIG_H.
add_definitions(-DHAVE_SYSTEM_H)
# Make sure config.h can be found.
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# Where the wild things are...
add_subdirectory(Mucipher)
add_subdirectory(Museekal)
add_subdirectory(Tools)
add_subdirectory(Muhelp)
add_subdirectory(Museek)

add_subdirectory(museekd)

project(museek-plus)

cmake_minimum_required(VERSION 2.6.0)

# Include some cmake checks.
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckCXXSourceCompiles)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
MESSAGE("DECLARATIONS: PREFIX (/usr), MANDIR (PREFIX/man)
Package options: EVERYTHING, NO_MUSEEKD, NO_MUSCAN, NO_SETUP, NO_MUSEEQ, NO_PYMUCIPHER, BINDINGS, MURMUR, MUCOUS, CLIENTS
Museeq options: BINRELOC, DATADIR (PREFIX/share), RELOAD_TRANSLATIONS
Prefix these declarations with -D and set them with either 1, 0 or a path.
Set -DEVERYTHING=1 if you want it all")

# Enable NewNet's smart-pointer debugging capabilities.
add_definitions(-DNN_PTR_DEBUG -DNN_PTR_DEBUG_ASSERT)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE(PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(PREFIX)

IF (NOT DATADIR)
  set(DATADIR ${CMAKE_INSTALL_PREFIX}/share)
ENDIF (NOT DATADIR)

set(MANUAL_DIRECTORY "man/man1")
if (MANDIR)
  set(MANUAL_DIRECTORY ${MANDIR}/man1)
endif (MANDIR)

set(PYMUCOUS 1)
IF (NOT MUCOUS)
  set(PYMUCOUS 0)
ENDIF (NOT MUCOUS)

set(PYMURMUR 1)
IF (NOT MURMUR)
  set(PYMURMUR 0)
ENDIF (NOT MURMUR)

set(PYCLIENTS 1)
IF (NOT CLIENTS)
  set(PYCLIENTS 0)
ENDIF (NOT CLIENTS)

set(PYBIND 1)
IF (NOT BINDINGS)
  set(PYBIND 0)
ENDIF (NOT BINDINGS)

IF (EVERYTHING)
  set(PYMUCIPHER 1)
  set(PYBIND 1)
  set(PYCLIENTS 1)
  set(PYMURMUR 1)
  set(PYMUCOUS 1)
  set(PYSETUP 1)
  set(C_MUSEEKD 1)
  set(C_MUSCAN 1)
  set(C_MUSEEQ 1)
ENDIF (EVERYTHING)

set(PYSETUP 1)
IF (NO_SETUP)
  set(PYSETUP 0)
ENDIF (NO_SETUP)

set(C_MUSEEKD 1)
IF (NO_MUSEEKD)
  set(C_MUSEEKD 0)
ENDIF (NO_MUSEEKD)

set(C_MUSCAN 1)
IF (NO_MUSCAN)
  set(C_MUSCAN 0)
ENDIF (NO_MUSCAN)

set(PYMUCIPHER 1)
IF (NO_PYMUCIPHER)
  set(PYMUCIPHER 0)
ENDIF (NO_PYMUCIPHER)

set(C_MUSEEQ 1)
IF (NO_MUSEEQ)
  set(C_MUSEEQ 0)
ENDIF (NO_MUSEEQ)

set(PYSETUP 1)
IF (NO_SETUP)
  set(PYSETUP 0)
ENDIF (NO_SETUP)

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
    return 0; }
" )
check_cxx_source_compiles("${code}" DIR_H)
if (NOT DIR_H)
    message(FATAL_ERROR "Headers for DIRENT not found")
endif (NOT DIR_H)

check_cxx_source_compiles("
  ${INCLUDE_HEAD}
  int main() {
    struct dirent **temp;
    scandir(0, &temp, 0, 0);
    return 0; }
" DIR_SCANDIR_H)
if (NOT DIR_SCANDIR_H)
    set(HAVE_SCANDIR 0)
endif (NOT DIR_SCANDIR_H)

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

# Process config.h.cmake to config.h.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
# Add -DHAVE_CONFIG_H.
add_definitions(-DHAVE_CONFIG_H)
# Make sure config.h can be found.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Set up new net libraries variable.
set(NEWNET_LIBRARIES NewNet)
if(WIN32 AND NOT CYGWIN)
  set(NEWNET_LIBRARIES ${NEWNET_LIBRARIES} wsock32)
endif(WIN32 AND NOT CYGWIN)

# Where the wild things are...

add_subdirectory(NewNet)
add_subdirectory(Mucipher)
if (C_MUSCAN)
  add_subdirectory(Muhelp)
  add_subdirectory(muscan)
else (C_MUSCAN)
  message("!!! muscan will NOT be installed [NO_MUSCAN=0]")
endif (C_MUSCAN)
if (C_MUSEEKD)
  if (NOT C_MUSCAN)
    add_subdirectory(Muhelp)
  endif (NOT C_MUSCAN)
  add_subdirectory(museekd)
else (C_MUSEEKD)
  message("!!! museekd will NOT be installed [NO_MUSEEKD=0]")
endif (C_MUSEEKD)
if (PYBIND)
  add_subdirectory(python-bindings)
else (PYBIND)
  message("!!! Python Bindings will NOT be installed.. [BINDINGS=1]")
endif (PYBIND)
if (PYCLIENTS)
  add_subdirectory(python-clients)
else (PYCLIENTS)
  message("!!! mulog, museekchat, museekcontrol, musIRC will NOT be installed [CLIENTS=1]")
endif (PYCLIENTS)
if (PYSETUP)
  add_subdirectory(setup)
else (PYSETUP)
  message("!!! musetup, musetup-gtk, musetup-qt will NOT be installed [NO_SETUP=0]")
endif (PYSETUP)
if (PYMUCOUS)
  add_subdirectory(mucous)
else (PYMUCOUS)
  message("!!! Mucous (Py/Curses Client)will NOT be installed.. [MUCOUS=1]")
endif (PYMUCOUS)
if (PYMURMUR)
  add_subdirectory(murmur)
else (PYMURMUR)
  message("!!! Murmur (PyGTK2 Client) will NOT be installed.. [MURMUR=1]")
endif (PYMURMUR)
if (C_MUSEEQ)
  add_subdirectory(museeq)
else (C_MUSEEQ)
  message("!!! museeq will NOT be installed [NO_MUSEEQ=0]")
endif (C_MUSEEQ)


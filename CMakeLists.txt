project(museek-plus)

cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Include some cmake checks.
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

# Enable NewNet's smart-pointer debugging capabilities.
add_definitions(-DNN_PTR_DEBUG -DNN_PTR_DEBUG_ASSERT)

message("DECLARATIONS:
Package options: EVERYTHING, MURMUR, MUCOUS, CLIENTS, BINDINGS
Museeq options: BINRELOC, RELOAD_TRANSLATIONS
Prefix these declarations with -D and set them with either 1, 0 or a path.
Set -DEVERYTHING=1 if you want it all.")

set(MUSEEKD ON CACHE BOOL "Build museekd soulseek daemon.")
set(MUSETUP ON CACHE BOOL "Build musetup configuration interface for museekd.")
set(MUSCAN ON CACHE BOOL "Build muscan shared file index generation tool.")
set(MUSEEQ ON CACHE BOOL "Build museeq Qt client.")

if(EVERYTHING)
    set(OPTIONAL_DEFAULT ON)
    set(FORCE_CACHE FORCE)
else()
    set(OPTIONAL_DEFAULT OFF)
endif()
unset(EVERYTHING CACHE)

set(MUCOUS ${OPTIONAL_DEFAULT} CACHE BOOL
    "Build mucous curses client." ${FORCE_CACHE})
set(MURMUR ${OPTIONAL_DEFAULT} CACHE BOOL
    "Build murmur PyGTK client." ${FORCE_CACHE})
set(PYMUCIPHER ${OPTIONAL_DEFAULT} CACHE BOOL
    "Generate PyMucipher bindings for Mucipher." ${FORCE_CACHE})
set(PYTHON_BINDINGS ${OPTIONAL_DEFAULT} CACHE BOOL
    "Generate python bindings." ${FORCE_CACHE})
set(PYTHON_CLIENTS ${OPTIONAL_DEFAULT} CACHE BOOL
    "Build python clients (mulog, museekchat, museekcontrol, musirc)."
     ${FORCE_CACHE})

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
else()
    message(FATAL_ERROR "Couldn't determine 32bit unsigned interger type.")
endif()

# Check for iconv.h
check_include_files(iconv.h HAVE_ICONV_H)
if(NOT HAVE_ICONV_H)
    message(FATAL_ERROR "Could not find iconv.h.")
endif()

# Check for iconv library (if we need it)
check_function_exists(iconv HAVE_ICONV_BUILTIN)

if(HAVE_ICONV_BUILTIN)
    set(ICONV_LIBRARIES "")
else()
    find_library(ICONV_LIBRARIES NAMES iconv PATHS /usr/lib /usr/local/lib)
    if(NOT ICONV_LIBRARIES)
        message(FATAL_ERROR "Couldn't find iconv library.")
    endif()
endif()

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
endif()

# Check if signal type.
check_cxx_source_compiles("
  #include <signal.h>
  void test_handler(int s) { };
  int main() {
     signal(11, test_handler); }
" SIGNAL_TYPE)

if(NOT SIGNAL_TYPE)
    set(RSIGTYPE 1)
endif()

# Clean up after iconv tests.
set(CMAKE_REQUIRED_LIBRARIES)

set(INCLUDE_HEAD "")
if(HAVE_DIRENT_H)
    set(INCLUDE_HEAD  "${INCLUDE_HEAD}#include <dirent.h>\n")
endif()

if(HAVE_SYS_DIR_H)
    set(INCLUDE_HEAD "${INCLUDE_HEAD}#include <sys/dir.h>\n")
endif()

if(HAVE_SYS_NDIR_H)
    set(INCLUDE_HEAD "${INCLUDE_HEAD}#include <sys/ndir.h>\n")
endif()

if(HAVE_NDIR_H)
    set(INCLUDE_HEAD "${INCLUDE_HEAD}#include <ndir.h>\n")
endif()

set(code "
  ${INCLUDE_HEAD}
  int main() {
    struct dirent **temp;
    return 0; }
" )
check_cxx_source_compiles("${code}" DIR_H)
if(NOT DIR_H)
    message(FATAL_ERROR "Headers for DIRENT not found")
endif()

check_cxx_source_compiles("
  ${INCLUDE_HEAD}
  int main() {
    struct dirent **temp;
    scandir(0, &temp, 0, 0);
    return 0; }
" DIR_SCANDIR_H)
if(NOT DIR_SCANDIR_H)
    set(HAVE_SCANDIR 0)
endif()

# Detect Vorbisfile library
if(HAVE_VORBIS_H)
    find_library(VORBIS_LIBRARIES NAMES vorbisfile PATHS /usr/lib /usr/local/lib)
    if(VORBIS_LIBRARIES)
        set(HAVE_VORBIS 1)
        message(STATUS "Vorbisfile library found at ${VORBIS_LIBRARIES}.")
    else()
        message(STATUS "Vorbisfile library wasn't found, ogg metadata will not be created")
        set(VORBIS_LIBRARIES "")
    endif()
else()
    set(VORBIS_LIBRARIES "")
endif()

set(CMAKE_REQUIRED_LIBRARIES)

if(NOT HAVE_STDLIB_H)
    message(FATAL_ERROR "STDLIB not found")
endif()
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
endif()

# Where the wild things are...

add_subdirectory(NewNet)
add_subdirectory(Mucipher)

if(MUSEEKD OR MUSCAN)
    add_subdirectory(Muhelp)
endif()

if(MUSEEKD)
    add_subdirectory(museekd)
else()
    message("!!! museekd will NOT be installed.")
endif()

if(MUSETUP)
    add_subdirectory(setup)
else()
    message("!!! musetup, musetup-gtk, musetup-qt will NOT be installed.")
endif()

if(MUSCAN)
    add_subdirectory(muscan)
else()
    message("!!! muscan will NOT be installed.")
endif()

if(MUSEEQ)
    add_subdirectory(museeq)
else()
    message("!!! museeq will NOT be installed.")
endif()

if(MUCOUS)
    add_subdirectory(mucous)
else()
    message("!!! mucous (Py/Curses Client)will NOT be installed.")
endif()

if(MURMUR)
    add_subdirectory(murmur)
else()
    message("!!! murmur (PyGTK2 Client) will NOT be installed.")
endif()

if(PYTHON_BINDINGS)
    add_subdirectory(python-bindings)
else()
    message("!!! Python bindings will NOT be installed.")
endif()

if(PYTHON_CLIENTS)
    add_subdirectory(python-clients)
else()
    message("!!! Python clients (mulog, museekchat, museekcontrol, musirc) will NOT be installed.")
endif()

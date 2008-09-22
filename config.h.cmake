#ifndef CONFIG_H
#define CONFIG_H

#cmakedefine HAVE_STDLIB_H 1
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_STDDEF_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_STRING_H 1
#cmakedefine HAVE_ERRNO_H 1
#cmakedefine HAVE_FCNTL_H 1
#cmakedefine HAVE_NETDB_H 1
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_SYS_TIME_H 1
#cmakedefine HAVE_SYS_SELECT_H 1
#cmakedefine HAVE_SYS_SOCKET_H 1
#cmakedefine HAVE_SYS_UN_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_NETINET_IN_H 1
#cmakedefine HAVE_NETINET_TCP_H 1
#cmakedefine HAVE_WINDOWS_H 1
#cmakedefine HAVE_WINSOCK_H 1

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#define _LARGE_FILES 1

#cmakedefine ICONV_CONST const
#ifndef ICONV_CONST
# define ICONV_CONST
#endif // ! ICONV_CONST
#define ICONV_INPUT_TYPE ICONV_CONST char **

#cmakedefine HAVE_UINT
#ifndef HAVE_UINT
 typedef unsigned int uint;
#endif

#cmakedefine HAVE_UINT32
#ifndef HAVE_UINT32
 typedef @UINT32_TYPE@ uint32;
 #define HAVE_UINT32
#endif

#endif // CONFIG_H

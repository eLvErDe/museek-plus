#ifndef SYSTEM_H
#define SYSTEM_H

#cmakedefine HAVE_STDLIB_H 1
#cmakedefine HAVE_STDIO_H 1
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_STDDEF_H 1
#cmakedefine HAVE_STDARG_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_STRING_H 1
#cmakedefine HAVE_STRINGS_H 1
#cmakedefine HAVE_ERRNO_H 1
#cmakedefine HAVE_FCNTL_H 1
#cmakedefine HAVE_NETDB_H 1
#cmakedefine HAVE_MEMORY_H 1
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_SYS_TIME_H 1
#cmakedefine HAVE_SYS_SOCKET_H 1
#cmakedefine HAVE_SYS_POLL_H 1
#cmakedefine HAVE_SYS_UN_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_EPOLL_CTL 1
#cmakedefine HAVE_SIGNAL_H 1
#cmakedefine HAVE_NETINET_IN_H 1
#cmakedefine HAVE_NETINET_TCP_H 1
#cmakedefine HAVE_WINDOWS_H 1
#cmakedefine HAVE_WINSOCK_H 1
#cmakedefine HAVE_PWD_H 1
#cmakedefine HAVE_SYSLOG_H 1
#cmakedefine HAVE_DIRENT_H 1
#cmakedefine HAVE_SYS_DIR_H 1
#cmakedefine HAVE_NDIR_H 1
#cmakedefine HAVE_SYS_NDIR_H 1
#cmakedefine HAVE_FAM_H 1
#cmakedefine HAVE_VORBIS 1
#ifdef HAVE_SYS_POLL_H
 #include <sys/poll.h>
#endif

#ifdef HAVE_SYS_TIME_H
 #include <time.h>
#endif

#ifdef HAVE_INTTYPES_H
 #include <inttypes.h>
#endif

#ifdef HAVE_DIRENT_H
 #include <dirent.h>
 #define NAMELEN(dirent) strlen((dirent)->d_name
 #define HAVE_SCANDIR 1
 #define SCANDIR_ENTRY dirent
#endif
#ifndef HAVE_DIRENT_H

#endif /* ! HAVE_DIRENT_H */

#ifdef HAVE_PWD_H
 #include <pwd.h>
#endif

#ifdef HAVE_SYSLOG_H
 #include <syslog.h>
#endif

#ifdef HAVE_STDIO_H
 #include <stdio.h>
#endif
#ifdef HAVE_STDINT_H
 #include <stdint.h>
#endif
#ifdef HAVE_STDDEF_H
 #include <stddef.h>
#endif
#ifdef HAVE_STDARG_H
 #include <stdarg.h>
#endif
#ifdef HAVE_UNISTD_H
 #include <unistd.h>
#endif
#ifdef HAVE_STRING_H
 #include <string.h>
#endif
#ifdef HAVE_STRINGS_H
 #include <strings.h>
#endif
#ifdef HAVE_ERRNO_H
 #include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
 #include <fcntl.h>
#endif
#ifdef HAVE_NETDB_H
 #include <netdb.h>
#endif
#ifdef HAVE_SYS_TYPES_H
 #include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
 #include <sys/time.h>
#endif
#ifdef HAVE_EPOLL_CTL
 #include <sys/epoll.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
 #include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
 #include <sys/un.h>
#endif
#ifdef HAVE_NETINET_IN_H
 #include <netinet/in.h>
#endif

#ifdef HAVE_SIGNAL_H
 #include <signal.h>
#endif

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#define _LARGE_FILES 1

#ifndef UNIX_PATH_MAX
 #define UNIX_PATH_MAX 108
#endif /* UNIX_PATH_MAX */

#cmakedefine ICONV_CONST const
#ifndef ICONV_CONST
# define ICONV_CONST
#endif /* ! ICONV_CONST */
#define ICONV_IN ICONV_CONST char **

#cmakedefine RSIGTYPE
#ifdef RSIGTYPE
# define RETSIGTYPE int
#endif /* RSIGTYPE */
#ifndef RSIGTYPE
# define RETSIGTYPE void
#endif /* ! RSIGTYPE */

#cmakedefine HAVE_UINT
#ifndef HAVE_UINT
 typedef unsigned int uint;
#endif

#cmakedefine HAVE_UINT32
#ifndef HAVE_UINT32
 typedef @UINT32_TYPE@ uint32;
 #define HAVE_UINT32
#endif

#endif /* SYSTEM_H */

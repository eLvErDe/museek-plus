#ifndef SYSTEM_H
#define SYSTEM_H

#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDARG_H 1
#define HAVE_UNISTD_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_ERRNO_H 1
#ifdef TRAYICON
 #define HAVE_TRAYICON 1
#endif

#define HAVE_SYS_SOCKET_H 1

#define HAVE_SYS_UN_H 1


#ifdef HAVE_SYS_TIME_H
 #include <time.h>
#endif

#ifdef HAVE_INTTYPES_H
 #include <inttypes.h>
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

#ifdef HAVE_SYS_SOCKET_H
 #include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
 #include <sys/un.h>
#endif


#ifdef HAVE_SIGNAL_H
 #include <signal.h>
#endif


#define HAVE_UINT
#ifndef HAVE_UINT
 typedef unsigned int uint;
#endif

/* #undef HAVE_UINT32 */
#ifndef HAVE_UINT32
 typedef unsigned int uint32;
 #define HAVE_UINT32
#endif

#endif // SYSTEM_H

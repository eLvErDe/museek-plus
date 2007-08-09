
#ifndef SYSTEM_H
#define SYSTEM_H

#include <errno.h>
typedef unsigned int uint32;


#ifdef HAVE_SYS_UN_H // UNIX
#include <sys/un.h>
#define HAVE_SYS_SOCKET_H 1
#include <sys/socket.h>
#endif // UNIX

#endif

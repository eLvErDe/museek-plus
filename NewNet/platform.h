/*  NewNet - A networking framework in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifndef NEWNET_PLATFORM_H
#define NEWNET_PLATFORM_H

#ifdef HAVE_CONFIG_H

# include "config.h"
# ifdef HAVE_STRING_H
#  include <string.h>
# endif // HAVE_STRING_H
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif // HAVE_SYS_TIME_H
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif // HAVE_SYS_TYPES_H
# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif // HAVE_SYS_SELECT_H
# ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif // HAVE_SYS_SOCKET_H
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif // HAVE_NETINET_IN_H
# ifdef HAVE_NETINET_TCP_H
#  include <netinet/tcp.h>
# endif // HAVE_NETINET_TCP_H
# ifdef HAVE_SYS_UN_H
#  include <sys/un.h>
# endif // HAVE_SYS_UN_H
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif // HAVE_NETDB_H
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif // HAVE_UNISTD_H
# ifdef HAVE_FCNTL_H
#  include <fcntl.h>
# endif // HAVE_FCNTL_H
# ifdef HAVE_ERRNO_H
#  include <errno.h>
# endif // HAVE_ERRNO_H
# ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
# endif // HAVE_SYS_STAT_H
# ifdef HAVE_WINDOWS_H
#  include <windows.h>
# endif // HAVE_WINDOWS_H
# ifdef HAVE_WINSOCK_H
#  include <winsock.h>
# endif // HAVE_WINSOCK_H

#else // HAVE_CONFIG_H

# include <string.h>
# include <sys/time.h>
# include <sys/types.h>
# ifndef WIN32
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <sys/un.h>
#  include <netdb.h>
# endif // ! WIN32
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <sys/stat.h>
# ifdef WIN32
#  include <windows.h>
#  include <winsock.h>
# endif // WIN32
#endif

#ifndef UNIX_PATH_MAX
# define UNIX_PATH_MAX 108
#endif

#ifndef WIN32
  typedef int sockopt_t;
# define closesocket close
# define WSAGetLastError() errno
# define WSAEWOULDBLOCK EINPROGRESS
#else
  typedef char sockopt_t;
  typedef int socklen_t;
#endif // ! WIN32

#endif // NEWNET_PLATFORM_H

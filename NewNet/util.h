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

#ifndef NEWNET_UTIL_H
#define NEWNET_UTIL_H

#include "platform.h"

static inline long
difftime(const struct timeval & a, const struct timeval & b)
{
  long diff_s = a.tv_sec - b.tv_sec;
  long diff_u;
  diff_u = a.tv_usec - b.tv_usec;
  if(diff_u < 0)
  {
    diff_s -= 1;
    diff_u += 1000000;
  }
  else if(diff_u >= 1000000)
  {
    diff_s += 1;
    diff_u -= 1000000;
  }
  return (diff_s * 1000) + (diff_u / 1000);
}

#ifdef NN_NO_GETTIMEOFDAY
// This piece of code came from a post in the CURL mailing list
static inline int
gettimeofday(struct timeval * tv, void * tz)
{
  union
  {
    LONGLONG ns100;
    FILETIME ft;
  } now;
  GetSystemTimeAsFileTime(&now.ft);
  tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
  tv->tv_sec = (long)((now.ns100 - 116444736000000000LL) / 1000000LL);
  return 0;
}
#endif // NN_NO_GETTIMEOFDAY

inline static bool
setnonblocking(int sock)
{
#ifndef WIN32
  int mode = fcntl(sock, F_GETFL, 0);
  if (fcntl(sock, F_SETFL, mode|O_NONBLOCK) < 0)
      return false;
#else
  u_long ioctlArg = 1;
  ioctlsocket(sock, FIONBIO, &ioctlArg);
#endif // ! WIN32
  return true;
}

#endif // NEWNET_UTIL_H

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

#include "nnunixserversocket.h"
#include "nnlog.h"
#include "platform.h"
#include <iostream>

void
NewNet::UnixServerSocket::listen(const std::string & path)
{
  if(path.length() >= UNIX_PATH_MAX)
  {
    NNLOG("newnet.net.warn", "Unix socket path too long: '%s'.", path.c_str());
    setSocketError(ErrorInvalidPath);
    cannotListenEvent(this);
  }

  unlink(path.c_str());

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));

  address.sun_family = AF_UNIX;
  memcpy(address.sun_path, path.c_str(), path.length());

  int sock = socket(PF_UNIX, SOCK_STREAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);

  mode_t old_umask = umask(0177);
  int ret = bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_un));
  umask(old_umask);
  if(ret != 0)
  {
    NNLOG("newnet.net.warn", "Cannot bind unix socket to '%s', error: %i.", path.c_str(), errno);
    closesocket(sock);
    setSocketError(ErrorCannotBind);
    cannotListenEvent(this);
    return;
  }

  if (::listen(sock, 3) != 0)
  {
    NNLOG("newnet.net.warn", "Cannot listen on unix socket '%s', error: %i.", path.c_str(), errno);
    closesocket(sock);
    setSocketError(ErrorCannotListen);
    cannotListenEvent(this);
    return;
  }

  m_Path = path;
  setDescriptor(sock);
  setSocketState(SocketListening);
  listeningEvent(this);

  NNLOG("newnet.net.debug", "Listening on unix socket '%s'.", path.c_str());
}

void
NewNet::UnixServerSocket::disconnect()
{
  if((descriptor() == -1) || (socketState() != SocketListening))
  {
    NNLOG("newnet.net.warn", "Trying to disconnect an uninitialized unix server socket.");
    return;
  }
  ServerSocket::disconnect();
  unlink(m_Path.c_str());
}

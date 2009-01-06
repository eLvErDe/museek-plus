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

#include "nntcpserversocket.h"
#include "nnlog.h"
#include "platform.h"
#include "util.h"
#include <iostream>

void
NewNet::TcpServerSocket::listen(const std::string & host, unsigned int port)
{
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));

  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if(! host.empty())
  {
    struct hostent *h = gethostbyname(host.c_str());
    if (! h)
    {
      NNLOG("newnet.net.warn", "Cannot resolve '%s'.", host.c_str());
      setSocketError(ErrorCannotResolve);
      cannotListenEvent(this);
      return;
    }
    memcpy(&(address.sin_addr.s_addr), *(h->h_addr_list), sizeof(address.sin_addr.s_addr));
  }
  else
    address.sin_addr.s_addr = INADDR_ANY;

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (!setnonblocking(sock))
    NNLOG("newnet.net.warn", "Couldn't set socket %i to non blocking (errno: %i)", sock, errno);

  sockopt_t socket_option = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(int));

  if(bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) != 0)
  {
    NNLOG("newnet.net.warn", "Cannot bind to '%s:%u', error: %i.", host.c_str(), port, errno);
    closesocket(sock);
    setSocketError(ErrorCannotBind);
    cannotListenEvent(this);
    return;
  }

  if (::listen(sock, 3) != 0)
  {
    NNLOG("newnet.net.warn", "Cannot listen on '%s:%u', error: %i.", host.c_str(), port, errno);
    closesocket(sock);
    setSocketError(ErrorCannotListen);
    cannotListenEvent(this);
    return;
  }

  m_ListenPort = port;
  setDescriptor(sock);
  setSocketState(SocketListening);
  listeningEvent(this);

  NNLOG("newnet.net.debug", "Listening on socket '%s:%u'.", host.c_str(), port);
}

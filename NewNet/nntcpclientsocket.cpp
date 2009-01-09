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

#include "nntcpclientsocket.h"
#include "nnlog.h"
#include "nnreactor.h"
#include "platform.h"
#include "util.h"
#include <iostream>

void
NewNet::TcpClientSocket::connect(const std::string & host, unsigned int port)
{
  assert((descriptor() == -1) || (socketState() == SocketUninitialized));

  setSocketState(SocketConnecting);

  NNLOG("newnet.net.debug", "Resolving host '%s'.", host.c_str());
  struct hostent *h = gethostbyname(host.c_str());
  if(! h)
  {
    NNLOG("newnet.net.warn", "Cannot resolve host '%s'.", host.c_str());
    setSocketError(ErrorCannotResolve);
    cannotConnectEvent(this);
    return;
  }

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  memcpy(&(address.sin_addr.s_addr), *(h->h_addr_list), sizeof(address.sin_addr.s_addr));
  address.sin_port = htons(port);

  NNLOG("newnet.net.debug", "Connecting to host '%s:%u'.", host.c_str(), port);

  int s = socket(PF_INET, SOCK_STREAM, 0);
  if (!setnonblocking(s))
    NNLOG("newnet.net.warn", "Couldn't set socket %i to non blocking (errno: %i)", s, errno);
  setDescriptor(s);

  if(s < 0)
    {
      NNLOG("newnet.net.warn", "Cannot connect to host '%s:%u', error: %i.", host.c_str(), port, WSAGetLastError());
      setSocketError(ErrorCannotConnect);
      cannotConnectEvent(this);
      return;
    }

  // Add a connection timeout
  if (reactor()) {
    m_ConnectionTimeout = reactor()->addTimeout(120000, this, &TcpClientSocket::onConnectionTimeout);
  }

  connectedEvent.connect(this, &TcpClientSocket::onConnected);

  if(::connect(s, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) == 0)
  {
    // When using non blocking socket (most of the time), we don't get here.
    NNLOG("newnet.net.debug", "Connected to host '%s:%u'.", host.c_str(), port);
    setSocketState(SocketConnected);
    connectedEvent(this);
  }
  else if(WSAGetLastError() != WSAEWOULDBLOCK)
  {
    // When using non blocking socket (most of the time), we don't get here.
    NNLOG("newnet.net.warn", "Cannot connect to host '%s:%u', error: %i.", host.c_str(), port, WSAGetLastError());
    setSocketError(ErrorCannotConnect);
    cannotConnectEvent(this);
  }
}

void
NewNet::TcpClientSocket::onConnectionTimeout(long) {
    cannotConnectEvent(this);
}

void
NewNet::TcpClientSocket::onConnected(ClientSocket *) {
  if (reactor())
    reactor()->removeTimeout(m_ConnectionTimeout);
}

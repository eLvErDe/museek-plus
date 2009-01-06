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

#include "nnserversocket.h"
#include "nnlog.h"
#include "platform.h"
#include "util.h"
#include <iostream>

void
NewNet::ServerSocket::disconnect()
{
  if((descriptor() == -1) || (socketState() != SocketListening))
  {
    NNLOG("newnet.net.warn", "Trying to disconnect an uninitialized server socket.");
    return;
  }

  closesocket(descriptor());
  setSocketState(SocketDisconnected);
  disconnectedEvent(this);
}

void
NewNet::ServerSocket::process()
{
  if (readyState() & StateReceive)
  {
    int client = ::accept(descriptor(), 0, 0);

    if(client == -1)
    {
      if ((WSAGetLastError() == EAGAIN) || (WSAGetLastError() == EMFILE))
        setReadyState(readyState() & ~StateReceive);
      else
        NNLOG("newnet.net.warn", "Ignoring error '%i' in ServerSocket::accept().", errno);
    }
    else {
      if (!setnonblocking(client))
        NNLOG("newnet.net.warn", "Couldn't set socket %i to non blocking (errno: %i)", client, errno);
      acceptedEvent(client);
    }
  }
}

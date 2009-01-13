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

#include "nnclientsocket.h"
#include "nnlog.h"
#include "platform.h"
#include <iostream>

void
NewNet::ClientSocket::disconnect(bool invoke)
{
  if((socketState() == SocketUninitialized) || (descriptor() < 0))
  {
    NNLOG("newnet.net.warn", "Trying to disconnect an uninitialized client socket.");
    if (invoke)
      disconnectedEvent(this);
    return;
  }

  closesocket(descriptor());
  setSocketState(SocketDisconnected);
  if (invoke)
    disconnectedEvent(this);
}

void
NewNet::ClientSocket::process()
{
  if(socketState() == SocketConnecting)
  {
    if(readyState() & StateSend)
    {
      sockopt_t so_error;
      socklen_t so_len = sizeof(int);
      getsockopt(descriptor(), SOL_SOCKET, SO_ERROR, &so_error, &so_len);
      if(so_len != sizeof(int) || ! so_error)
      {
        NNLOG("newnet.net.debug", "Connected to host");
        setSocketState(SocketConnected);
        connectedEvent(this);
      }
      else
      {
        NNLOG("newnet.net.warn", "Cannot connect to host, error: %i.", so_error);
        setSocketError(ErrorCannotConnect);
        cannotConnectEvent(this);
        return;
      }
    }
  }

  if(readyState() & StateException)
  {
    unsigned char buf;
    ssize_t received = ::recv(descriptor(), (char *)&buf, 1, MSG_OOB);
    if(received < 1)
    {
      NNLOG("newnet.net.warn", "Socket %u encountered error %i. Closing it.", descriptor(), errno);
      closesocket(descriptor());
      setSocketError(ErrorUnknown);
      disconnectedEvent(this);
      return;
    }
    m_ReceiveBuffer.append(&buf, 1);
    dataReceivedEvent(this);
  }

  if(readyState() & StateReceive)
  {
    unsigned char buf[1024];
    ssize_t received = ::recv(descriptor(), (char *)buf, 1024, 0);
    if(received == -1)
    {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        NNLOG("newnet.net.debug", "EAGAIN while receiving data on socket %i.", descriptor());
        setReadyState(readyState() & ~StateReceive);
      }
      else
      {
        NNLOG("newnet.net.warn", "Socket %u encountered error %i. Closing it.", descriptor(), errno);
        closesocket(descriptor());
        setSocketError(ErrorUnknown);
        disconnectedEvent(this);
        return;
      }
    }
    else if(received == 0)
    {
      NNLOG("newnet.net.debug", "Socket %u was disconnected.", descriptor());
      closesocket(descriptor());
      setSocketState(SocketDisconnected);
      disconnectedEvent(this);
      return;
    }
    else
    {
      NNLOG("newnet.net.debug", "Received %i bytes on socket %u.", received, descriptor());
      if(downRateLimiter())
        downRateLimiter()->transferred(received);
      m_ReceiveBuffer.append(buf, received);
      dataReceivedEvent(this);
    }
  }

  if((readyState() & StateSend) && dataWaiting())
  {
    size_t n = std::min((size_t)1024, m_SendBuffer.count());
    ssize_t sent = ::send(descriptor(), (const char *)m_SendBuffer.data(), n, 0);
    if(sent < 0)
    {
      if(errno == EAGAIN)
        setReadyState(readyState() & ~StateSend);
      else
      {
        NNLOG("newnet.net.warn", "Socket %u encountered error %i. Closing it.", descriptor(), errno);
        closesocket(descriptor());
        setSocketError(ErrorUnknown);
        disconnectedEvent(this);
        return;
      }
    }
    else {
        NNLOG("newnet.net.debug", "Sent %i bytes to socket %u.", sent, descriptor());
        if(upRateLimiter())
          upRateLimiter()->transferred(sent);
        m_SendBuffer.seek(sent);
        setDataWaiting(m_SendBuffer.count() != 0);
        dataSentEvent(this);
    }
  }
}

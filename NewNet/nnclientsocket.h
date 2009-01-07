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

#ifndef NEWNET_CLIENTSOCKET_H
#define NEWNET_CLIENTSOCKET_H

#include "nnsocket.h"
#include "nnbuffer.h"
#include "nnevent.h"

namespace NewNet
{
  //! Base class for network client sockets.
  /*! Provides a base class that will monitor the state of the socket and
      emit events when something happens. Note: this class is usually not
      used by your application. Applications should use TcpClientSocket or
      UnixClientSocket instead. */
  class ClientSocket : public Socket
  {
  public:
    //! Create an empty client socket.
    /*! This will create an empty client socket. The client socket starts in
        an uninitialized state without a descriptor. */
    ClientSocket() : Socket()
    {
    }

    //! Disconnect the client socket.
    /*! This immediately disconnects the client socket and invokes the
        disconnected event (except if invoke is false). */
    virtual void disconnect(bool invoke = true);

    //! Process network events.
    /*! This method is called by the reactor, it will process the events
        specified in readyState(): read from the socket, detect disconnections
        and send data through the socket. */
    virtual void process();

    //! Append data to the send buffer.
    /*! This is a convenience function that will append data to the send
        buffer and will mark the flag that specifies that the socket wants
        to send data. */
    void send(const unsigned char * data, size_t n)
    {
      m_SendBuffer.append(data, n);
      setDataWaiting(m_SendBuffer.count() > 0);
    }

    //! Return a reference to the send buffer.
    /*! Returns a reference to the send buffer. Note: if you manipulate the
        send buffer, be sure to call setDataWaiting(bool) to make sure the
        data waiting flag is set correctly. */
    Buffer & sendBuffer()
    {
      return m_SendBuffer;
    }

    //! Return a reference to the receive buffer.
    /*! Returns a reference to the receive buffer. */
    Buffer & receiveBuffer()
    {
      return m_ReceiveBuffer;
    }

    //! Invoked when the socket can't connect.
    /*! This event will be invoked when the socket detects it cannot
        connect to the remote host. */
    Event<ClientSocket *> cannotConnectEvent;
    //! Invoked when the socket is connected.
    /*! This event will be invoked when the socket detects it has
        successfully connected to the remote host. */
    Event<ClientSocket *> connectedEvent;
    //! Invoked when the socket is disconnected.
    /*! This event will be invoked when the socket has been disconnected
        from the remote host. */
    Event<ClientSocket *> disconnectedEvent;
    //! Invoked when there's new data waiting in the receive buffer.
    /*! This event will be invoked when the socket received information
        from the remote host. The information will be appended to the
        receive buffer. */
    Event<ClientSocket *> dataReceivedEvent;
    //! Invoked when data has been sent through the network socket.
    /*! This event will be invoked when data has been send to the remote
        host. */
    Event<ClientSocket *> dataSentEvent;

  private:
    Buffer m_SendBuffer, m_ReceiveBuffer;
  };
}

#endif // NEWNET_CLIENTSOCKET_H

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

#ifndef NEWNET_SERVERSOCKET_H
#define NEWNET_SERVERSOCKET_H

#include "nnsocket.h"
#include "nnevent.h"
#include <string>

namespace NewNet
{
  //! Base class for network server sockets.
  /*! Provides a base class that will monitor the state of the socket and
      emit events when something happens. Note: this class is usually not
      used by your application. Applications should use TcpServerSocket,
      UnixServerSocket or one of the associated factories instead. */
  class ServerSocket : public Socket
  {
  public:
    //! Create an empty server socket.
    /*! This will create an empty server socket. The server socket starts in
        an uninitialized state without a descriptor. */
    ServerSocket() : Socket()
    {
    }

    //! Disconnect the server socket.
    /*! Closes the server socket and clean up any remaining resources. */
    virtual void disconnect();

    //! Process network events.
    /*! Gets called by the reactor detects a new connection attempt on the
        server socket. When that happens, accept() is called. */
    virtual void process();

    //! Emitted when the socket can't start listening.
    /*! Subclasses emit this event when there's an error when it's attempting
        to start to listen. */
    Event<ServerSocket *> cannotListenEvent;
    //! Emitted when the socket starts listening.
    /*! Subclasses emit this when they start listening. */
    Event<ServerSocket *> listeningEvent;
    //! Emitted when a client has been accepted.
    /*! Emitted when a client has been accepted. The argument is the
        descriptor for the new client. */
    Event<int> acceptedEvent;
    //! Emitted when the server socket has been closed.
    /*! Emitted when the server socket has been closed. */
    Event<ServerSocket *> disconnectedEvent;
  };
}

#endif // NEWNET_SERVERSOCKET_H

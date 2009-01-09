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

#ifndef NEWNET_TCPCLIENTSOCKET_H
#define NEWNET_TCPCLIENTSOCKET_H

#include "nnclientsocket.h"
#include <string>

namespace NewNet
{
  //! TCP/IP client socket.
  /*! A client socket that provides a streaming internet (TCP/IP) socket
      type. */
  class TcpClientSocket : public ClientSocket
  {
  public:
    //! Create a new unconnected TCP/IP client socket.
    /*! This creates a new, unconnected TCP/IP client socket. Call connect()
        to connect the client socket to a remote host. */
    TcpClientSocket() : ClientSocket()
    {
    }

    //! Connect to a remote host.
    /*! Creates a new descriptor and tries to connect it to the specified
        port on the specified host. */
    void connect(const std::string & host, unsigned int port);

    void onConnectionTimeout(long);

    void onConnected(ClientSocket *);

  private:
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_ConnectionTimeout;
  };
}

#endif // NEWNET_TCPCLIENTSOCKET_H

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

#ifndef NEWNET_TCPSERVERSOCKET_H
#define NEWNET_TCPSERVERSOCKET_H

#include "nnserversocket.h"

namespace NewNet
{
  //! Implements a TCP/IP server class.
  /*! Implements a TCP/IP server class that can listen for incoming
      connections on a specified host and port. */
  class TcpServerSocket : public ServerSocket
  {
  public:
    //! Create a TCP/IP server socket.
    /*! Creates a TCP/IP server socket. The socket isn't yet bound to
        anything, so call listen() to activate the server socket. */
    TcpServerSocket() : ServerSocket(), m_ListenPort(0)
    {
    }

    //! Listen on a port on the specified host.
    /*! Starts listening on a port on the specified host. */
    void listen(const std::string & host, unsigned int port);

    //! Listen on a port of any active network adapter. */
    /*! Starts listening on the specified port of all active network
        adapters. */
    void listen(unsigned int port)
    {
      listen(std::string(), port);
    }

    //! Return listen port
    /*! Return which port this server socket is listening on. */
    unsigned int listenPort()
    {
      return m_ListenPort;
    }

  private:
    unsigned int m_ListenPort;
  };
}

#endif // NEWNET_TCPSERVERSOCKET_H

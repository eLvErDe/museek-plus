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

#ifndef NEWNET_FACTORYSOCKET_H
#define NEWNET_FACTORYSOCKET_H

#include "nnreactor.h"

namespace NewNet
{
  //! Abstract base class for factory sockets.
  /*! This provides a base class for factory sockets: server sockets that
      automatically create a specified client socket type. Note: your
      application will most likely use TcpFactorySocket or UnixFactorySocket
      instead of using this class directly. */
  template<class ServerType, class ClientType>
  class FactorySocket : public Object
  {
  public:
    //! Construct a factory socket.
    /*! Create a new factory socket by instantiating a new socket of type
        ServerType using the default constructor. Note: stores a RefPtr
        to the newly created ServerType. */
    FactorySocket()
    {
      m_ServerSocket = new ServerType;
      m_ServerSocket->acceptedEvent.connect(this, &FactorySocket::buildClient);
    }

    //! Construct a factory socket based on the specified server socket.
    /*! Create a new factory socket based on the the specified server
        socket. Note: this stores a RefPtr to the specified server socket. */
    FactorySocket(ServerType * serverSocket)
    {
      m_ServerSocket = serverSocket;
      m_ServerSocket->acceptedEvent.connect(this, &FactorySocket::buildClient);
    }

    //! Returns the server socket.
    /*! Use this to make the factory socket listen on a port or path (depending
        on the server socket type used) or to add it to the reactor. */
    ServerType * serverSocket()
    {
      return m_ServerSocket;
    }

    //! Invoked when a new client socket has been created.
    /*! This event is invoked when a new client socket has been created. Note:
        you don't have to add the created client to the reactor. FactorySocket
        does that for you. */
    Event<ClientType *> clientAcceptedEvent;

  private:
    void buildClient(int descriptor)
    {
      ClientType * client = new ClientType();
      client->setDescriptor(descriptor);
      client->setSocketState(Socket::SocketConnected);
      if(m_ServerSocket->reactor())
        m_ServerSocket->reactor()->add(client);
      clientAcceptedEvent(client);
    }

    NewNet::RefPtr<ServerType> m_ServerSocket;
  };
}

#endif // NEWNET_FACTORYSOCKET_H

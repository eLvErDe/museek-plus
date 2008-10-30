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

#ifndef NEWNET_UNIXFACTORYSOCKET_H
#define NEWNET_UNIXFACTORYSOCKET_H

#include "nnunixserversocket.h"
#include "nnfactorysocket.h"

namespace NewNet
{
  //! Unix factory socket class.
  /*! This provides your application with a factory socket that can listen
      on a path and create UnixClientSockets. */
  template<class ClientType>
  class UnixFactorySocket : public FactorySocket<UnixServerSocket, ClientType>
  {
  public:
    //! Create a new unix factory socket.
    /*! Create a new unix factory socket. Don't forget to call
        listen on serverSocket() and add serverSocket() to the reactor. */
    UnixFactorySocket() : FactorySocket<UnixServerSocket, ClientType>()
    {
    }
  };
}

#endif // NEWNET_UNIXFACTORYSOCKET_H

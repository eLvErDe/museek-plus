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

#ifndef NEWNET_UNIXSERVERSOCKET_H
#define NEWNET_UNIXSERVERSOCKET_H

#include "nnserversocket.h"

namespace NewNet
{
  //! Implements a unix socket server class.
  /*! Implements a unix socket server class that can listen for incoming
      connections on a specified path. */
  class UnixServerSocket : public ServerSocket
  {
  public:
    //! Create a unix server socket.
    /*! Creates a unix server socket. The socket isn't yet bound to
        anything, so call listen() to activate the server socket. */
    UnixServerSocket() : ServerSocket()
    {
    }

    //! Listen on a unix path.
    /*! Starts listening on the specified path. */
    void listen(const std::string & path);

    //! Disconnect the server socket.
    /*! Closes the server socket and removes the unix socket file. */
    virtual void disconnect();

  private:
    std::string m_Path;
  };
}

#endif // NEWNET_UNIXSERVERSOCKET_H

/*  Museek - A SoulSeek client written in C++
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

#ifndef MUSEEK_HANDSHAKESOCKET_H
#define MUSEEK_HANDSHAKESOCKET_H

#include <NewNet/nnclientsocket.h>
#include <NewNet/nnweakrefptr.h>
#include "messageprocessor.h"

namespace Museek
{
  class Museekd;

  /* HandshakeSocket is used when a new incoming connection has been
     established. HandshakeSocket reads one message then creates the
     appropriate socket type re-using the same descriptor. */
  class HandshakeSocket : public NewNet::ClientSocket, public MessageProcessor
  {
  public:
    HandshakeSocket();
    ~HandshakeSocket();

    void setMuseekd(Museekd * museekd);
    Museekd * museekd() const
    {
      return m_Museekd;
    }

    /* Return the token the remote peer sent. */
    uint token() const
    {
      return m_Token;
    }

    /* Return the username the remote peer claims it is. */
    const std::string & user() const
    {
      return m_User;
    }

  private:
    /* Handler for messageReceivedEvent. */
    void onMessageReceived(const MessageData * data);
    void onDisconnected(NewNet::ClientSocket * socket);
    void onCannotConnect(NewNet::ClientSocket * socket);

    NewNet::WeakRefPtr<Museekd> m_Museekd;
    uint m_Token;
    std::string m_User;
  };
}

#endif // MUSEEK_HANDSHAKESOCKET_H

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

#ifndef MUSEEK_USERSOCKET_H
#define MUSEEK_USERSOCKET_H

#include <NewNet/nntcpclientsocket.h>
#include "servermessages.h"
#include "handshakesocket.h"

namespace Museek
{
  class Museekd;

  class UserSocket : public NewNet::TcpClientSocket
  {
  public:
    UserSocket(Museekd * museekd, const std::string & type);
    UserSocket(HandshakeSocket * that, const std::string & type);
    ~UserSocket();

    void initiate(const std::string & user);
    void reverseConnect(const std::string & user, uint token, const std::string & ip, uint port);

    Museekd * museekd() const { return m_Museekd; }

    const std::string & type() const { return m_Type; }

    uint token() const { return m_Token; }
    void setToken(uint token) { m_Token = token; }

    const std::string & user() const { return m_User; }
    void setUser(const std::string & user) { m_User = user; }

    void sendMessage(const NewNet::Buffer & buffer);

    void firewallPierced(HandshakeSocket * socket);

  protected:
    void initiateActive();
    void initiatePassive();

    void onServerPeerAddressReceived(const SGetPeerAddress * message);
    void onCannotReverseConnect(NewNet::ClientSocket *);
    void onFirewallPierceTimedOut(long);
    void onDisconnected(NewNet::ClientSocket *);
    void onCannotConnectNotify(const SCannotConnect * msg);

  private:
    NewNet::WeakRefPtr<Museekd> m_Museekd;
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_PassiveConnectTimeout;

    std::string m_Type, m_User;
    uint m_Token;
  };
}

#endif // MUSEEK_USERSOCKET_H

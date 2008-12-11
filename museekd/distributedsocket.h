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

#ifndef MUSEEK_DISTRIBUTEDSOCKET_H
#define MUSEEK_DISTRIBUTEDSOCKET_H

#include "usersocket.h"
#include "messageprocessor.h"
#include "distributedmessages.h"

namespace Museek
{
  class DistributedSocket : public UserSocket, public MessageProcessor
  {
  public:
    DistributedSocket(HandshakeSocket * that);
    DistributedSocket(Museekd * museekd);
    ~DistributedSocket();

    void sendPosition();

    void initiateActiveWithIP(const std::string & user, const std::string & ip, uint port);

    void ping(long);
    void setPingTimeout(NewNet::WeakRefPtr<NewNet::Event<long>::Callback> ref) {m_PingTimeout = ref;};

    void addDisconnectNowTimeout();
    void onDisconnectNow(long);

    void stop();

    #define MAP_MESSAGE(ID, TYPE, EVENT) NewNet::Event<const TYPE *> EVENT;
    #include "distributedeventtable.h"
    #undef MAP_MESSAGE

  private:
    void onMessageReceived(const MessageData * data);
    void onBranchLevelReceived(const DBranchLevel * msg);
    void onBranchRootReceived(const DBranchRoot * msg);
    void onChildDepthReceived(const DChildDepth * msg);
    void onSearchRequested(const DSearchRequest * msg);
    void onCannotConnectActive(NewNet::ClientSocket * socket);
    void onFirewallPierceTimedOut(long);
    void onDisconnected(NewNet::ClientSocket * socket);
    void onConnected(NewNet::ClientSocket * socket);

    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_PingTimeout;
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_DisconnectNowTimeout;
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_DataTimeout;
  };
}

#endif // MUSEEK_DISTRIBUTEDSOCKET_H

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H
#include "handshakesocket.h"
#include "museekd.h"
#include "handshakemessages.h"
#include "peersocket.h"
#include "peermanager.h"
#include "ticketsocket.h"
#include "distributedsocket.h"
#include "searchmanager.h"
#include <NewNet/nnreactor.h>

Museek::HandshakeSocket::HandshakeSocket() : NewNet::ClientSocket(), Museek::MessageProcessor(1)
{
  // Connect some signals.
  dataReceivedEvent.connect(this, &HandshakeSocket::onDataReceived);
  messageReceivedEvent.connect(this, &HandshakeSocket::onMessageReceived);
  disconnectedEvent.connect(this, &HandshakeSocket::onDisconnected);
  cannotConnectEvent.connect(this, &HandshakeSocket::onCannotConnect);
}

Museek::HandshakeSocket::~HandshakeSocket()
{
  NNLOG("museekd.hand.debug", "HandshakeSocket %d destroyed", descriptor());
}

void
Museek::HandshakeSocket::setMuseekd(Museekd * museekd)
{
  m_Museekd = museekd;
}

void
Museek::HandshakeSocket::onMessageReceived(const MessageData * data)
{
  switch(data->type)
  {
    case 0:
    {
      // Apparently, we requested somebody to connect to us.
      NNLOG("museek.messages.handshake", "Received peer handshake message HPierceFirewall");
      HPierceFirewall msg;
      msg.parse_network_packet(data->data, data->length);
      m_Token = msg.token;
      receiveBuffer().seek(data->length + 5);
      // Tell the peer manager, it should know more.
      m_Museekd->peers()->firewallPiercedEvent(this);
      // This particular socket is no longer needed. Remove it from the reactor.
      reactor()->remove(this);
      return;
    }
    case 1:
    {
      // Somebody wants to establish a connection.
      NNLOG("museek.messages.handshake", "Received peer handshake message HInitiate");
      HInitiate msg;
      msg.parse_network_packet(data->data, data->length);
      NNLOG("museekd.hand.debug", "HInitiate payload: %s %s %u", msg.user.c_str(), msg.type.c_str(), msg.token);
      // Set some variables.
      m_Token = msg.token;
      m_User = msg.user;
      // Seek past the message.
      if (receiveBuffer().count() >= data->length + 5)
        receiveBuffer().seek(data->length + 5);
      else
        receiveBuffer().clear();
      if(msg.type == "P")
      {
        // Create a new PeerSocket which will copy our descriptor and state.
        Museek::PeerSocket * that = new Museek::PeerSocket(this);
        museekd()->peers()->addPeerSocket(that);
        // Add the newly constructed socket to the reactor.
        reactor()->add(that);
      }
      else if(msg.type == "F")
      {
        // Create a new TicketSocket which will copy our descriptor and state.
        Museek::TicketSocket * that = new Museek::TicketSocket(this);
        // Add the newly constructed socket to the reactor.
        reactor()->add(that);
        // There may be some data waiting in the buffer (sent at connection). We have to ask the ticketsocket to check it.
        that->findTicket();
      }
      else if(msg.type == "D")
      {
        // Create a new DistributedSocket which will copy our descriptor and state.
        Museek::DistributedSocket * that = new Museek::DistributedSocket(this);
        // A potential parent doesn't care about our position
        if (!museekd()->searches()->isPotentialParent(m_User))
            that->sendPosition();
        // Add the newly constructed socket to the reactor.
        reactor()->add(that);
      }
      else
      {
        NNLOG("museekd.hand.warn", "Invalid incoming connection type '%s'.", msg.type.c_str());
      }
      // Clear our receive buffer so we stop processing data.
      receiveBuffer().clear();
      // Remove this socket from the reactor as it's no longer needed.
      reactor()->remove(this);
      return;
    }
    default:
        NNLOG("museekd.hand.warn", "Received unknown peer handshake message, type: %u, length: %u", data->type, data->length);
        NetworkMessage msg;
        msg.parse_network_packet(data->data, data->length);
  }
}

void
Museek::HandshakeSocket::onDisconnected(NewNet::ClientSocket * socket) {
  NNLOG("museekd.hand.debug", "Handshake socket for %s has been disconnected.", m_User.c_str());
  if(reactor())
    reactor()->remove(this);
}

void
Museek::HandshakeSocket::onCannotConnect(NewNet::ClientSocket *)
{
  NNLOG("museekd.hand.debug", "Could not connect handshake socket for user %s.", m_User.c_str());
  disconnect();
}

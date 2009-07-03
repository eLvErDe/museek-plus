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
#include "distributedsocket.h"
#include "museekd.h"
#include "searchmanager.h"
#include "handshakemessages.h"
#include "servermanager.h"
#include "codesetmanager.h"
#include <NewNet/nnreactor.h>

Museek::DistributedSocket::DistributedSocket(Museek::HandshakeSocket * that) : Museek::UserSocket(that, "D"), Museek::MessageProcessor(1)
{
    messageReceivedEvent.connect(this, &DistributedSocket::onMessageReceived);
    dataReceivedEvent.connect(this, &TcpMessageSocket::onDataReceived);
    branchLevelReceivedEvent.connect(this, &DistributedSocket::onBranchLevelReceived);
    branchRootReceivedEvent.connect(this, &DistributedSocket::onBranchRootReceived);
    childDepthReceivedEvent.connect(this, &DistributedSocket::onChildDepthReceived);
    searchRequestedEvent.connect(this, &DistributedSocket::onSearchRequested);
    disconnectedEvent.connect(this, &DistributedSocket::onDisconnected);
    connectedEvent.connect(this, &DistributedSocket::onConnected);
}

Museek::DistributedSocket::DistributedSocket(Museek::Museekd * museekd) : Museek::UserSocket(museekd, "D"), Museek::MessageProcessor(1)
{
    messageReceivedEvent.connect(this, &DistributedSocket::onMessageReceived);
    dataReceivedEvent.connect(this, &TcpMessageSocket::onDataReceived);
    branchLevelReceivedEvent.connect(this, &DistributedSocket::onBranchLevelReceived);
    branchRootReceivedEvent.connect(this, &DistributedSocket::onBranchRootReceived);
    childDepthReceivedEvent.connect(this, &DistributedSocket::onChildDepthReceived);
    searchRequestedEvent.connect(this, &DistributedSocket::onSearchRequested);
    disconnectedEvent.connect(this, &DistributedSocket::onDisconnected);
    connectedEvent.connect(this, &DistributedSocket::onConnected);
}

Museek::DistributedSocket::~DistributedSocket()
{
    if (m_DisconnectNowTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_DisconnectNowTimeout);

    if (m_DataTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_DataTimeout);

    NNLOG("museekd.distrib.debug", "DistributedSocket destroyed");
}

/**
  * Sends our position in the soulseek search tree
  */
void Museek::DistributedSocket::sendPosition() {
    DBranchLevel msgL(museekd()->searches()->branchLevel());
    sendMessage(msgL.make_network_packet());

    DBranchRoot msgR(museekd()->searches()->branchRoot());
    sendMessage(msgR.make_network_packet());
}

void
Museek::DistributedSocket::initiateActiveWithIP(const std::string & user, const std::string & ip, uint port)
{
    setUser(user);
    setToken(museekd()->token());

    NNLOG("museekd.distrib.debug", "Initiating active distributed connection to %s (type %s, ip %s, port %d).", user.c_str(), type().c_str(), ip.c_str(), port);

    HInitiate handshake(museekd()->server()->username(), type(), token());
    sendMessage(handshake.make_network_packet());

    cannotConnectEvent.connect(this, & DistributedSocket::onCannotConnectActive);

    if((ip == "0.0.0.0") || (port == 0)) {
        cannotConnectEvent(this);
        return;
    }
    connect(ip, port);
}

/**
  * Ping the peer and launch a timer for the next ping
  */
void
Museek::DistributedSocket::ping(long) {
    DPing msg;
    sendMessage(msg.make_network_packet());
    m_PingTimeout = museekd()->reactor()->addTimeout(60000, this, &DistributedSocket::ping);
}

void
Museek::DistributedSocket::onConnected(NewNet::ClientSocket * socket) {
    // Check that the peer sends us something (if not, it's probably a child who has found another parent)
    if (m_DataTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_DataTimeout);

    m_DataTimeout = museekd()->reactor()->addTimeout(60000, this, &DistributedSocket::onDisconnectNow);
}

void
Museek::DistributedSocket::onDisconnected(NewNet::ClientSocket * socket) {
    // We have to remove our timeout
    museekd()->reactor()->removeTimeout(m_PingTimeout);
}

void Museek::DistributedSocket::onCannotConnectActive(NewNet::ClientSocket * socket) {
    NNLOG("museekd.distrib.debug", "Cannot connect a distributed socket in active mode. Trying passive.");
    socket->sendBuffer().clear(); // We have a HInitiate message still waiting in the buffer. We don't need it anymore
    disconnect();
    initiatePassive();
}

void
Museek::DistributedSocket::onFirewallPierceTimedOut(long)
{
    // Distributed socket tries first active mode, then passive (unlike usersocket).
    // So no need to retry active when passive fails.
    NNLOG("museekd.distrib.debug", "Passive connection failed: pierce firewall timed out.");

    disconnect();
}

void Museek::DistributedSocket::onBranchLevelReceived(const DBranchLevel * msg) {
    museekd()->searches()->branchLevelReceived(this, msg->level);
}

void Museek::DistributedSocket::onBranchRootReceived(const DBranchRoot * msg) {
    if (this == museekd()->searches()->parent())
        museekd()->searches()->setBranchRoot(msg->root);
}

void Museek::DistributedSocket::onChildDepthReceived(const DChildDepth * msg) {
    if (this != museekd()->searches()->parent())
        museekd()->searches()->setChild(this, msg->depth);
}

void Museek::DistributedSocket::onSearchRequested(const DSearchRequest * msg) {
    std::string query = museekd()->codeset()->fromNet(msg->query);

    NNLOG("museekd.distrib.debug", "Received search request from our parent: %s for %s", query.c_str(), msg->username.c_str());

    museekd()->searches()->transmitSearch(msg->unknown, msg->username, msg->ticket, query);
    museekd()->searches()->sendSearchResults(msg->username, query, msg->ticket);
}

void
Museek::DistributedSocket::onMessageReceived(const MessageData * data)
{
    if (m_DataTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_DataTimeout);

  switch(data->type)
  {
    #define MAP_MESSAGE(ID, TYPE, EVENT) \
      case ID: \
      { \
        NNLOG("museekd.messages.distributed", "Received distributed message " #TYPE "."); \
        TYPE msg; \
        msg.setDistributedSocket(this); \
        msg.parse_network_packet(data->data, data->length); \
        EVENT(&msg); \
        break; \
      }
    #include "distributedeventtable.h"
    #undef MAP_MESSAGE

    default:
        NNLOG("museekd.distrib.warn", "Received unknown distributed message, type: %u, length: %u", data->type, data->length);
        NetworkMessage msg;
        msg.parse_network_packet(data->data, data->length);
  }
}

void
Museek::DistributedSocket::addDisconnectNowTimeout() {
    if (m_DisconnectNowTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_DisconnectNowTimeout);
    m_DisconnectNowTimeout = museekd()->reactor()->addTimeout(1000, this, &DistributedSocket::onDisconnectNow);
}

void
Museek::DistributedSocket::onDisconnectNow(long) {
    stop();
}

/*
    Stops the socket
*/
void
Museek::DistributedSocket::stop()
{
    NNLOG("museekd.distrib.debug", "Disconnecting distributed socket...");
    disconnect();
}

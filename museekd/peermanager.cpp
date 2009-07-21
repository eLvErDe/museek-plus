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
#include "peermanager.h"
#include "museekd.h"
#include "handshakesocket.h"
#include "servermanager.h"
#include "peersocket.h"
#include "ticketsocket.h"
#include "distributedsocket.h"
#include "searchmanager.h"
#include "ifacemanager.h"
#include <NewNet/util.h>
#include <NewNet/nntcpserversocket.h>

Museek::PeerManager::PeerManager(Museekd * museekd) : m_Museekd(museekd)
{
  museekd->config()->keySetEvent.connect(this, &PeerManager::onConfigKeySet);
  museekd->config()->keyRemovedEvent.connect(this, &PeerManager::onConfigKeyRemoved);
  museekd->server()->loggedInStateChangedEvent.connect(this, &PeerManager::onServerLoggedInStateChanged);
  museekd->server()->cannotConnectNotifyReceivedEvent.connect(this, &PeerManager::onCannotConnectNotify);
  museekd->server()->connectToPeerRequestedEvent.connect(this, &PeerManager::onServerConnectToPeerRequested);
  museekd->server()->userStatusReceivedEvent.connect(this, &PeerManager::onServerUserStatusReceived);
  museekd->server()->addUserReceivedEvent.connect(this, &PeerManager::onServerAddUserReceived);

  firewallPiercedEvent.connect(this, &PeerManager::onFirewallPierced);

  listen();
}

Museek::PeerManager::~PeerManager()
{
}

void
Museek::PeerManager::unlisten()
{
    if(m_Factory.isValid()) {
        m_Factory->serverSocket()->disconnect();
        if (m_Factory->serverSocket()->reactor())
            m_Museekd->reactor()->remove(m_Factory->serverSocket());
    }
    m_Factory = 0;
}

void
Museek::PeerManager::listen()
{
  uint first = m_Museekd->config()->getUint("clients.bind", "first"),
       last =  m_Museekd->config()->getUint("clients.bind", "last");

  if((first == 0) || (first > last))
  {
    NNLOG("museekd.peers.warn", "No valid client bind port set (range: %i - %i).", first, last);
    unlisten();
    return;
  }

  if(m_Factory.isValid())
  {
    unsigned int port = m_Factory->serverSocket()->listenPort();
    if((port >= first) && (port <= last))
      return;
    unlisten();
  }

  unsigned int port = first;
  while(port <= last)
  {
    NNLOG("museekd.peers.debug", "Trying to bind to port %i...", port);
    m_Factory = new PeerFactory();
    m_Factory->clientAcceptedEvent.connect(this, &PeerManager::onClientAccepted);

    m_Museekd->reactor()->add(m_Factory->serverSocket());
    m_Factory->serverSocket()->listen(port);
    if(m_Factory->serverSocket()->socketState() == NewNet::Socket::SocketListening)
    {
      onServerLoggedInStateChanged(m_Museekd->server()->loggedIn());
      NNLOG("museekd.peers.debug", "Listening for peers on port %i", port);
      return;
    }
    unlisten();
    ++port;
  }

  NNLOG("museekd.peers.warn", "Couldn't find port to listen for peers on (range: %i - %i).", first, last);
}

/**
  * Returns a peersocket for the given user name.
  */
void
Museek::PeerManager::peerSocket(const std::string & user, bool force) {
    NNLOG("museekd.peers.debug", "Asking a peersocket for %s", user.c_str());

    // Check if this user is already registered.
    std::map<std::string, NewNet::WeakRefPtr<PeerSocket> >::iterator it;
    it = m_Peers.find(user);
    if(it == m_Peers.end() || !it->second) {
        // Nope, see if we can open a new peersocket
        int maxSocket = museekd()->reactor()->maxSocketNo();
        int currentSockets = museekd()->reactor()->currentSocketNo();

        if (!force && (maxSocket > 0) && (currentSockets > (maxSocket - static_cast<int>(maxSocket*0.5)))) {
            NNLOG("museekd.peers.warn", "Too many opened peer socket, cannot open a new one with low priority");
            peerSocketUnavailableEvent(user);
            return;
        }

        // We can, register the user
        m_Peers[user] = 0;

        if(museekd()->server()->loggedIn()) {
            std::map<std::string, uint32>::iterator it = m_UserStatus.find(user);
            if (it == m_UserStatus.end()) {
                NNLOG("museekd.peers.debug", "No peer socket to %s, requesting status.", user.c_str());
                requestUserData(user);
            }
            else if (isUserConnected(user)) {
                // We know user is connected to server, create the socket
                createPeerSocket(user);
            }
            else {
                // User is offline, cannot create the socket
                NNLOG("museekd.peers.debug", "User %s is offline", user.c_str());

                // Don't disconnect the peer socket if it exists: it will be disconnected if the user is completely offline.
                // If he's only disconnected from the server, we can still talk to him directly.
                peerOfflineEvent(user);
            }


        }
    }
    else
        peerSocketReadyEvent((*it).second);
}

/**
  * Ask the server to give us the given user existence, status and stats
  */
void Museek::PeerManager::requestUserData(const std::string& user) {
    // Tell the server we want to watch this user
    // We don't need to send another SAddUser if we've just sent one
    struct timeval now;
    gettimeofday(&now, NULL);
    std::map<std::string, struct timeval >::iterator tit;
    tit = m_LastStatusTime.find(user);
    if(tit == m_LastStatusTime.end() || difftime(now, (*tit).second) > 10000.0) {
        NNLOG("museekd.peers.debug", "Asking server for existence, status and stats of user %s", user.c_str());
        m_LastStatusTime[user] = now;

        // Let the server know we want to track the status of this user.
        SAddUser msg(user);
        museekd()->server()->sendMessage(msg.make_network_packet());
    }
}

/**
  * Register a peer socket associated with the given user
  */
void Museek::PeerManager::addPeerSocket(PeerSocket * socket) {
    if(socket->user() == std::string()) {
        NNLOG("museekd.peers.warn", "Cannot add a peer socket with no user associated.");
        return;
    }

    NNLOG("museekd.peers.debug", "Adding peer socket for %s", socket->user().c_str());

    bool isOurself = (socket->user() == museekd()->server()->username());

    // Disconnect and remove any existing peer socket for this user except if we're connecting to ourself
    if (!isOurself)
        removePeerSocket(socket->user(), true);

    // There shouldn't be several peer sockets for the same user. If that happens, keep only the last one.
    m_Peers[socket->user()] = socket;
    if (!isOurself || (isOurself && !m_Peers[socket->user()])) {
        // Don't watch for disconnection twice if we're trying to connect to ourself
        socket->cannotConnectEvent.connect(this, &PeerManager::onPeerCannotConnect);
        socket->disconnectedEvent.connect(this, &PeerManager::onDisconnected);
    }
    if (socket->socketState() == NewNet::Socket::SocketConnected)
        peerSocketReadyEvent(socket);
    else
        socket->connectedEvent.connect(this, &PeerManager::onConnected);
}

/**
  * Remove a peer socket associated with the given user if found. If disconnect is set to true, the peer socket will be disconnected first.
  */
void Museek::PeerManager::removePeerSocket(const std::string & user, bool disconnect) {
    std::map<std::string, NewNet::WeakRefPtr<PeerSocket> >::iterator it;
    it = m_Peers.find(user);
    if (it != m_Peers.end()) {
        if (disconnect && it->second)
            it->second->disconnect();
        else // disconnect will call removePeerSocket later with disconnect = false
            m_Peers.erase(it);
    }
}

/*
    The status of a user has changed
*/
void
Museek::PeerManager::onServerUserStatusReceived(const SGetStatus * message)
{
    setUserStatus(message->user, message->status); // Store status for future ifaces

    // Is the user online?
    if(message->status > 0) {
        NNLOG("museekd.peers.debug", "User %s is now online", message->user.c_str());

        createPeerSocket(message->user);
    }
    else {
        NNLOG("museekd.peers.debug", "User %s is now offline", message->user.c_str());

        peerOfflineEvent(message->user);
    }
}

/**
  *  Set the status of an user
  */
void
Museek::PeerManager::setUserStatus(const std::string& user, uint32 status)
{
    m_UserStatus[user] = status; // Store status for future ifaces
}

/**
  * Create a peer socket for the given user. You shouldn't call this directly, call peerSocket() first.
  */
void
Museek::PeerManager::createPeerSocket(const std::string& user) {
    // Only create a peer socket if we asked for it earlier
    std::map<std::string, NewNet::WeakRefPtr<PeerSocket> >::iterator it;
    it = m_Peers.find(user);
    if(it != m_Peers.end()) {
        // Check if we already have a socket for this user.
        PeerSocket * socket = m_Peers[user];
        if(! socket) {
            // Nope. Create a new one.
            socket = new PeerSocket(museekd());
            socket->setUser(user);
            addPeerSocket(socket);
            museekd()->reactor()->add(socket);
            if (user == museekd()->server()->username())
                socket->initiateOurself();
            else
                socket->initiate(user);
        }
    }
}

/**
  * We have received stats of an user
  */
void
Museek::PeerManager::onServerAddUserReceived(const SAddUser * message)
{
    if (!message->exists)
        return;

    m_UserStats[message->user] = message->userdata; // Store stats for future ifaces

    setUserStatus(message->user, message->userdata.status); // Store status for future ifaces

    // Is the user online?
    if(message->userdata.status > 0) {
        NNLOG("museekd.peers.debug", "User %s is online", message->user.c_str());

        createPeerSocket(message->user);
    }
    else {
        NNLOG("museekd.peers.debug", "User %s is offline", message->user.c_str());

        // Don't disconnect the peer socket if it exists: it will be disconnected if the user is completely offline.
        // If he's only disconnected from the server, we can still talk to him directly.
        peerOfflineEvent(message->user);
    }
}

/**
  * Returns true if the given user is known to be online by museekd. If the user is offline or is not known by us, returns false.
  */
bool Museek::PeerManager::isUserConnected(const std::string& user) {
    std::map<std::string, uint32>::iterator it = m_UserStatus.find(user);
    if (it == m_UserStatus.end())
        return false;
    else
        return (it->second > 0);
}

/*
    Called when the connection cannot be made with the peer.
*/
void
Museek::PeerManager::onPeerCannotConnect(NewNet::ClientSocket * socket_)
{
	NNLOG("museekd.peers.debug", "Cannot connect to the peer");
    // Cast the socket to a peer socket.
    PeerSocket * socket = (PeerSocket *)socket_;
    // Get the name of the user.
    std::string user = socket->user();

    socket->disconnect();

    peerSocketUnavailableEvent(user);
}

/*
    Called when the connection cannot be made with ourself (actively 127.0.0.1:listenport).
*/
void
Museek::PeerManager::onCannotConnectOurself(NewNet::ClientSocket * socket_) {
    // Could't connect actively, try the standard way (which needs reverse NAT)
    PeerSocket * socket = (PeerSocket *)socket_;
    if (socket) {
        socket->stopConnectOurself();
        socket->initiate(museekd()->server()->username());
    }
}

void
Museek::PeerManager::onCannotConnectNotify(const SCannotConnect * msg) {
	NNLOG("museekd.peers.debug", "Cannot connect to the peer %s", msg->user.c_str());

    peerSocketUnavailableEvent(msg->user);
}

/*
    Called when a peer socket gets disconnected
*/
void
Museek::PeerManager::onDisconnected(NewNet::ClientSocket * socket_)
{
	NNLOG("museekd.peers.debug", "Peer socket disconnected");

    // Cast the socket to a peer socket.
    PeerSocket * socket = (PeerSocket *)socket_;

    // Get the name of the user.
    std::string user(socket->user());
    // Check if this is the most recently constructed socket.
    removePeerSocket(user);
}

void
Museek::PeerManager::onConnected(NewNet::ClientSocket * socket_)
{
    // Cast the socket to a peer socket.
    PeerSocket * socket = (PeerSocket *)socket_;
    peerSocketReadyEvent(socket);
}

void
Museek::PeerManager::onClientAccepted(Museek::HandshakeSocket * socket)
{
  socket->setMuseekd(m_Museekd);
}

void
Museek::PeerManager::onConfigKeySet(const Museek::ConfigManager::ChangeNotify * data)
{
  if(data->domain == "clients.bind")
  {
    listen();
  }
}

void
Museek::PeerManager::onConfigKeyRemoved(const Museek::ConfigManager::RemoveNotify * data)
{
  if(data->domain == "clients.bind")
  {
    listen();
  }
}

void
Museek::PeerManager::onServerLoggedInStateChanged(bool loggedIn)
{
  if(loggedIn)
  {
    uint port = m_Factory ? m_Factory->serverSocket()->listenPort() : 0;
    m_Museekd->server()->sendMessage(SSetListenPort(port).make_network_packet());
  }
}

void
Museek::PeerManager::onServerConnectToPeerRequested(const SConnectToPeer * message)
{
    if (message->type == "P") {
        PeerSocket * socket = new PeerSocket(m_Museekd);
        socket->setUser(message->user);
        addPeerSocket(socket);
        m_Museekd->reactor()->add(socket);
        socket->reverseConnect(message->user, message->token, message->ip, message->port);
    }
    else if (message->type == "F") {
        TicketSocket * socket = new TicketSocket(m_Museekd);
        m_Museekd->reactor()->add(socket);
        socket->reverseConnect(message->user, message->token, message->ip, message->port);
        // There may be some data waiting in the buffer (sent at connection). We have to ask the ticketsocket to check it.
        socket->findTicket();
    }
    else if (message->type == "D") {
        // Create a new DistributedSocket which will copy our descriptor and state.
        DistributedSocket * socket = new DistributedSocket(m_Museekd);
        // A potential parent doesn't care about our position
        if (!museekd()->searches()->isPotentialParent(message->user))
            socket->sendPosition();
        m_Museekd->reactor()->add(socket);
        socket->reverseConnect(message->user, message->token, message->ip, message->port);
    }
}

/**
  * Register a usersocket that is waiting for a response from a peer during a passive connection.
  */
void
Museek::PeerManager::waitingPassiveConnection(UserSocket * socket) {
    if (socket && (socket->token() > 0)) {
        m_PassiveConnects[socket->token()] = socket;
    }
}

/**
  * Unregister a usersocket that is no longer waiting for a response from a peer during a passive connection.
  */
void
Museek::PeerManager::removePassiveConnectionWaiting(uint token) {
    if (token > 0) {
        std::map<uint, NewNet::RefPtr<UserSocket> >::iterator it;
        it = m_PassiveConnects.find(token);
        if (it != m_PassiveConnects.end())
            m_PassiveConnects.erase(it);
    }
}

void
Museek::PeerManager::onFirewallPierced(Museek::HandshakeSocket * socket)
{
    std::map<uint, NewNet::RefPtr<UserSocket> >::iterator it;
    it = m_PassiveConnects.find(socket->token());
    if (it != m_PassiveConnects.end()) {
        it->second->firewallPierced(socket);
        m_PassiveConnects.erase(it);
    }
    else {
        NNLOG("museekd.user.warn", "Received an unexpected HPierceFirewall message.");
        socket->disconnect(false);
    }
}

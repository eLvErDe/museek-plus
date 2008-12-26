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
  museekd->server()->userStatsReceivedEvent.connect(this, &PeerManager::onServerUserStatsReceived);
  museekd->reactor()->tooManySockets.connect(this, &PeerManager::onTooManySockets);
  museekd->reactor()->notTooManySockets.connect(this, &PeerManager::onNotTooManySockets);

  m_AllowConnections = true;

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

        if (!force && (maxSocket > 100) && ((currentSockets > (maxSocket - static_cast<int>(maxSocket*0.5))) || (museekd()->reactor()->maxFileDescriptor() > FD_SETSIZE - 500))) {
            NNLOG("museekd.peers.warn", "Too many opened peer socket, cannot open a new one with low priority");
            peerSocketUnavailableEvent(user);
            return;
        }

        // We can, register the user
        m_Peers[user] = 0;

        if(museekd()->server()->loggedIn()) {
            std::map<std::string, uint32>::iterator it = m_UserStatus.find(user);
            if (m_UserStatus.find(user) == m_UserStatus.end()) {
                // This is a new user, tell the server we want to watch him
                // We don't need to send another SGetStatus if we've just sent one
                struct timeval now;
                gettimeofday(&now, NULL);
                std::map<std::string, struct timeval >::iterator tit;
                tit = m_LastStatusTime.find(user);
                if(tit == m_LastStatusTime.end() || difftime(now, (*tit).second) > 10000.0) {
                    NNLOG("museekd.peers.debug", "No peer socket to %s, requesting status.", user.c_str());
                    m_LastStatusTime[user] = now;

                    // Let the server know we want to track the status of this user.
                    SAddUser msg(user);
                    museekd()->server()->sendMessage(msg.make_network_packet());

                    // Ask the server for this user's status. Once we get that a connection will be established.
                    SGetStatus msgS(user);
                    museekd()->server()->sendMessage(msgS.make_network_packet());
                }
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

/*
    Register a peer socket associated with the given user
*/
void Museek::PeerManager::addPeerSocket(PeerSocket * socket) {
    if(socket->user() == std::string()) {
        NNLOG("museekd.peers.warn", "Cannot add a peer socket with no user associated.");
        return;
    }

    NNLOG("museekd.peers.debug", "Adding peer socket for %s", socket->user().c_str());
    // There shouldn't be several peer sockets for the same user. If that happens, keep only the last one.
    m_Peers[socket->user()] = socket;
    socket->cannotConnectEvent.connect(this, &PeerManager::onPeerCannotConnect);
    socket->disconnectedEvent.connect(this, &PeerManager::onDisconnected);
    if (socket->socketState() == NewNet::Socket::SocketConnected)
        peerSocketReadyEvent(socket);
    else
        socket->connectedEvent.connect(this, &PeerManager::onConnected);
}

/*
    The status of a user has changed
*/
void
Museek::PeerManager::onServerUserStatusReceived(const SGetStatus * message)
{
    m_UserStatus[message->user] = message->status; // Store status for future ifaces

    // Is the user online?
    if(message->status > 0) {
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
  * Called when we have to stop creating new sockets
  */
void
Museek::PeerManager::onTooManySockets(int) {
    NNLOG("museekd.peers.warn", "Too many opened sockets, trying to unlisten.");
    museekd()->ifaces()->sendStatusMessage(true, std::string("Too many opened sockets. Unlistening. You may have problems connecting to other peers for a little while."));
    m_AllowConnections = false;
    unlisten();
}

/**
  * Called when we can create new sockets
  */
void
Museek::PeerManager::onNotTooManySockets(int) {
    NNLOG("museekd.peers.warn", "Some sockets were freed. Listening again.");
    museekd()->ifaces()->sendStatusMessage(true, std::string("Some sockets were freed. Listening again. Museek should now work perfectly."));
    m_AllowConnections = true;
    listen();
    museekd()->uploads()->checkUploads();
    museekd()->downloads()->checkDownloads();
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
            socket->initiate(user);
        }
    }
}

/**
  * We have received stats of an user
  */
void
Museek::PeerManager::onServerUserStatsReceived(const SGetUserStats * message)
{
    m_UserStats[message->user] = *message; // Store stats for future ifaces
}

/**
  * Returns true if the given user is known to be online by museekd. If the user is offline or is not known by us, returns false.
  */
bool Museek::PeerManager::isUserConnected(const std::string& user) {
    std::map<std::string, uint32>::iterator it = m_UserStatus.find(user);
    if (m_UserStatus.find(user) == m_UserStatus.end())
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
    // Check if this is the most recently constructed socket.
    std::map<std::string, NewNet::WeakRefPtr<PeerSocket> >::iterator it;
    it = m_Peers.find(user);
    if((it != m_Peers.end()) && (it->second == socket)) {
        socket->disconnect();
        if (socket->reactor())
            museekd()->reactor()->remove(socket);
    }

    peerSocketUnavailableEvent(user);
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
    std::string user = socket->user();
    // Check if this is the most recently constructed socket.
    std::map<std::string, NewNet::WeakRefPtr<PeerSocket> >::iterator it;
    it = m_Peers.find(user);
    if ((it != m_Peers.end()) && (it->second == socket))
        m_Peers.erase(it);
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
    if ((message->type == "P") && m_AllowConnections) {
        PeerSocket * socket = new PeerSocket(m_Museekd);
        socket->setUser(message->user);
        museekd()->peers()->addPeerSocket(socket);
        m_Museekd->reactor()->add(socket);
        socket->reverseConnect(message->user, message->token, message->ip, message->port);
    }
    else if (message->type == "F") { // It's important: try to connect even if m_AllowConnections is false
        TicketSocket * socket = new TicketSocket(m_Museekd);
        m_Museekd->reactor()->add(socket);
        socket->reverseConnect(message->user, message->token, message->ip, message->port);
        // There may be some data waiting in the buffer (sent at connection). We have to ask the ticketsocket to check it.
        socket->findTicket();
    }
    else if ((message->type == "D") && m_AllowConnections) {
        // Create a new DistributedSocket which will copy our descriptor and state.
        DistributedSocket * socket = new DistributedSocket(m_Museekd);
        // A potential parent doesn't care about our position
        if (!museekd()->searches()->isPotentialParent(message->user))
            socket->sendPosition();
        m_Museekd->reactor()->add(socket);
        socket->reverseConnect(message->user, message->token, message->ip, message->port);
    }
}

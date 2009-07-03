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
#include "searchmanager.h"
#include "museekd.h"
#include "codesetmanager.h"
#include "servermanager.h"
#include "peermanager.h"
#include "sharesdatabase.h"
#include "ifacemanager.h"
#include <NewNet/nnreactor.h>
#include <NewNet/util.h>

Museek::SearchManager::SearchManager(Museekd * museekd) : m_Museekd(museekd)
{
    // Connect some events.
    museekd->server()->loggedInStateChangedEvent.connect(this, &SearchManager::onServerLoggedInStateChanged);
    museekd->server()->netInfoReceivedEvent.connect(this, &SearchManager::onNetInfoReceived);
    museekd->server()->searchRequestedEvent.connect(this, &SearchManager::onSearchRequested);
    museekd->server()->fileSearchRequestedEvent.connect(this, &SearchManager::onFileSearchRequested);
    museekd->peers()->peerSocketReadyEvent.connect(this, &SearchManager::onPeerSocketReady);
    museekd->peers()->peerSocketUnavailableEvent.connect(this, &SearchManager::onPeerSocketUnavailable);
    museekd->server()->addUserReceivedEvent.connect(this, &SearchManager::onAddUserReceived);
    museekd->server()->wishlistIntervalReceivedEvent.connect(this, &SearchManager::onWishlistIntervalReceived);
    museekd->config()->keySetEvent.connect(this, &SearchManager::onConfigKeySet);
    museekd->config()->keyRemovedEvent.connect(this, &SearchManager::onConfigKeyRemoved);

    m_Parent = 0;
    m_ParentIp = std::string();
    m_BranchRoot = std::string();
    m_BranchLevel = 0;
    m_TransferSpeed = 0;
    m_ChildrenMaxNumber = 3;
    m_WishlistInterval = 720; // Default wishlist interval
}

Museek::SearchManager::~SearchManager()
{
    if (m_WishlistTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_WishlistTimeout);
    NNLOG("museekd.peers.debug", "Search Manager destroyed");
}

/**
  * Returns the maximum depth of our tree of children
  */
uint Museek::SearchManager::childDepth() {
    uint depth = 0;

    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, uint> >::const_iterator it;
    for (it = m_Children.begin(); it != m_Children.end(); it++) {
        uint d = it->second.second + 1;
        if (d > depth)
            depth = d;
    }

    return depth;
}

/**
  * Register or update a child with the given socket and his child depth
  */
void Museek::SearchManager::setChild(DistributedSocket * socket, uint depth) {
    if (socket) {
        bool isUpdate = m_Children.find(socket->user()) != m_Children.end();
        if ((parent() || isUpdate) && acceptChildren()) {
            NNLOG("museekd.peers.debug", "Setting child: %s, depth %d", socket->user().c_str(), depth);

            uint oldDepth = childDepth();

            m_Children[socket->user()] = std::pair<NewNet::RefPtr<DistributedSocket>, uint>(socket, depth);
            uint newDepth = childDepth();

            if (oldDepth != newDepth) {
                NNLOG("museekd.peers.debug", "Our child depth is now %d", newDepth);
                SChildDepth msg(newDepth);
                museekd()->server()->sendMessage(msg.make_network_packet());

                if (parent()) {
                    DChildDepth msgP(newDepth);
                    parent()->sendMessage(msgP.make_network_packet());
                }
            }

            if (!isUpdate) {
                socket->disconnectedEvent.connect(this, &SearchManager::onChildDisconnected);
                socket->setPingTimeout(museekd()->reactor()->addTimeout(60000, socket, &DistributedSocket::ping));

                // We have reached our maximum number of children
                if (m_Children.size() >= m_ChildrenMaxNumber) {
                    SAcceptChildren msgA(false);
                    museekd()->server()->sendMessage(msgA.make_network_packet());
                }
            }
        }
        else {
            // We want to disconnect this socket. BUT we have to be sure that there isn't any remaining handshakesocket in the reactor
            // before doing this (If there is still a handshake socket, it will stay in the reactor even if it's a dead socket so there
            // will be problems. This happens if the peer sends us both HInitiate and DChildDepth message at once.).
            // To be sure of that, wait 1 second before disconnecting
            socket->addDisconnectNowTimeout();
        }
    }
}

/**
  * Removes a child from our tree
  */
void Museek::SearchManager::removeChild(const std::string & user) {
    NNLOG("museekd.peers.debug", "Removing child: %s", user.c_str());

    uint oldDepth = childDepth();

    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, uint> >::iterator it = m_Children.find(user);
    if(it != m_Children.end()) {
        m_Children.erase(it);

        // If we have a new child depth, inform the server and our parent
        uint newDepth = childDepth();
        if (oldDepth != newDepth) {
            NNLOG("museekd.peers.debug", "Our child depth is now %d", newDepth);
            SChildDepth msg(newDepth);
            museekd()->server()->sendMessage(msg.make_network_packet());

            if (parent()) {
                DChildDepth msgP(newDepth);
                parent()->sendMessage(msgP.make_network_packet());
            }
        }

        // We have one more place for a children
        if (m_Children.size() == m_ChildrenMaxNumber - 1) {
            SAcceptChildren msgA(true);
            museekd()->server()->sendMessage(msgA.make_network_packet());
        }
    }
}

/**
  * Register a parent we're trying to connect to.
  */
void Museek::SearchManager::addPotentialParent(const std::string & user, DistributedSocket * socket, const std::string & ip) {
    m_PotentialParents[user].first = socket;
    m_PotentialParents[user].second = ip;
}

/**
  * We have found a parent: stop connecting to others
  */
void Museek::SearchManager::setParent(DistributedSocket * parentSocket) {
    if (parent() && parentSocket)
        parent()->stop();

    m_Parent = parentSocket;
    if (parentSocket) {
        std::string parentName = parentSocket->user();
        NNLOG("museekd.peers.debug", "Found a parent : %s", parentName.c_str());
        std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, std::string> >::iterator it;

        std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, std::string> > parents = m_PotentialParents;

        for (it = parents.begin(); it != parents.end(); it++) {
            if (it->first != parentName) {
                if (it->second.first.isValid())
                    it->second.first->stop();
            }
            else
                m_ParentIp = it->second.second;
            m_PotentialParents.erase(it->first);
        }

        SAcceptChildren msgAccept(true);
        museekd()->server()->sendMessage(msgAccept.make_network_packet());

        parentSocket->disconnectedEvent.connect(this, &SearchManager::onParentDisconnected);
        parentSocket->setPingTimeout(museekd()->reactor()->addTimeout(60000, parentSocket, &DistributedSocket::ping));
    }
}

/**
  * We have a new branch root, store it and notify the server
  */
void Museek::SearchManager::setBranchRoot(const std::string & root) {
    NNLOG("museekd.peers.debug", "Our branch root is %s", root.c_str());
    m_BranchRoot = root;

    // Tell our new parent how many children we've got
    DChildDepth msgD(childDepth());
    parent()->sendMessage(msgD.make_network_packet());

    // Send out parent IP to the server
    SParentIP msgIp(m_ParentIp);
    museekd()->server()->sendMessage(msgIp.make_network_packet());
    // Send our branch level to the server
    SBranchLevel msgL(branchLevel());
    museekd()->server()->sendMessage(msgL.make_network_packet());
    // Send the new branch root to the server
    SBranchRoot msgR(root);
    museekd()->server()->sendMessage(msgR.make_network_packet());

    // Tell our children what we've done
    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, uint> >::iterator cit;
    for (cit = m_Children.begin(); cit != m_Children.end(); cit++) {
        if (cit->second.first.isValid())
            cit->second.first->sendPosition();
    }
}

/**
  * The server sends us a parent list.
  */
void Museek::SearchManager::onNetInfoReceived(const SNetInfo * msg) {
    if (parent())
        setParent(0);

    std::map<std::string, std::pair<std::string, uint32> >::const_iterator it;
    for (it = msg->users.begin(); it != msg->users.end(); it++) {
        NNLOG("museekd.peers.debug", "Potential parent: %s (%s:%i)", it->first.c_str(), it->second.first.c_str(), it->second.second);

        DistributedSocket * socket = new DistributedSocket(museekd());
        museekd()->reactor()->add(socket);
        addPotentialParent(it->first, socket, it->second.first);
        socket->initiateActiveWithIP(it->first, it->second.first, it->second.second); // Don't ask for ip/port: we already know it
    }
}

/**
  * Called when some potential parent sends us his branch level
  */
void Museek::SearchManager::branchLevelReceived(DistributedSocket * socket, uint level) {
    // The first potential parent who gives us his level will be our official parent.

    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, std::string> >::iterator it;
    it = m_PotentialParents.find(socket->user());

    if (!parent() && it != m_PotentialParents.end()) {
        setParent(socket);
        setBranchLevel(level++);
        SHaveNoParents msg(false);
        museekd()->server()->sendMessage(msg.make_network_packet());
    }
    else if (parent() == socket) {
        // This is an update from our parent
        setBranchLevel(level++);
    }
}

/**
  * The server sends us a search request directly
  */
void Museek::SearchManager::onSearchRequested(const SSearchRequest * msg) {
    std::string query = museekd()->codeset()->fromNet(msg->query);

    NNLOG("museekd.peers.debug", "Received search request from server: %s for %s", query.c_str(), msg->username.c_str());

    transmitSearch(msg->unknown, msg->username, msg->token, msg->query);
    sendSearchResults(msg->username, query, msg->token);
}

/**
  * The server sends us a search request directly, outside the distributed system
  */
void Museek::SearchManager::onFileSearchRequested(const SFileSearch * msg) {
    std::string query = museekd()->codeset()->fromNet(msg->query);

    NNLOG("museekd.peers.debug", "Received file search request from server: %s for %s", query.c_str(), msg->user.c_str());

    sendSearchResults(msg->user, query, msg->ticket);
}

/**
  * The server sends us a SAddUser response: see if there's something interesting
  */
void Museek::SearchManager::onAddUserReceived(const SAddUser * msg) {
    if (msg->user == museekd()->server()->username()) {
        NNLOG("museekd.peers.debug", "Our transfer speed is %d", msg->userdata.avgspeed);
        setTransferSpeed(msg->userdata.avgspeed);
    }
}

/**
  * The server sends us the wishlist interval
  */
void Museek::SearchManager::onWishlistIntervalReceived(const SWishlistInterval * msg) {
    NNLOG("museekd.peers.debug", "New wishlist interval: %d", msg->value);
    m_WishlistInterval = msg->value;
    if (m_WishlistTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_WishlistTimeout);
    m_WishlistTimeout = museekd()->reactor()->addTimeout(m_WishlistInterval*1000, this, &SearchManager::onWishlistTimeout);
}

/**
  * We can send a new wishlist request
  */
void Museek::SearchManager::onWishlistTimeout(long) {
    if (!m_Wishlist.empty()) {
        // Which request should we do?
        std::map<std::string, time_t>::iterator oldest = m_Wishlist.begin(), it = m_Wishlist.begin();
        for (; it != m_Wishlist.end(); it++) {
            if (it->second < oldest->second)
                oldest = it;
        }
        NNLOG("museekd.peers.debug", "Sending wishlist search for '%s'", oldest->first.c_str());
        uint token = museekd()->token();

        museekd()->ifaces()->sendNewSearchToAll(oldest->first, token);
        // Send query
        SWishlistSearch msg(token, museekd()->codeset()->toNet(oldest->first));
        museekd()->server()->sendMessage(msg.make_network_packet());
        // Update item's last query date
        museekd()->config()->set("wishlist", oldest->first, static_cast<uint>(time(NULL)));
    }

    // Prepare next wishlist timeout
    m_WishlistTimeout = museekd()->reactor()->addTimeout(m_WishlistInterval*1000, this, &SearchManager::onWishlistTimeout);
}

/**
  * Send the given search request to our children
  * The query's encoding should be the network one
  */
void Museek::SearchManager::transmitSearch(uint unknown, const std::string & username, uint ticket, const std::string & query) {
    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, uint> >::const_iterator it;
    for (it = m_Children.begin(); it != m_Children.end(); it++) {
        DistributedSocket * socket = it->second.first;
        if (socket) {
            DSearchRequest msgD(unknown, username, ticket, query);
            socket->sendMessage(msgD.make_network_packet());
        }
    }
}

/**
  * Send the results from a query to the asker
  * The query's encoding should be UTF-8
  */
void Museek::SearchManager::sendSearchResults(const std::string & username, const std::string & query, uint token) {
	if(! museekd()->isBanned(username) && (username != museekd()->server()->username())) {
        SharesDatabase* db;

        if (museekd()->isBuddied(username))
            db = museekd()->buddyshares();
        else
            db = museekd()->shares();

        Folder results;
        db->search(query, results);

        if (!results.empty()) {
            m_PendingResults[username][token] = results;
            museekd()->peers()->peerSocket(username, false);
        }
	}
}

/**
  * Initiate a search in our buddy list
  */
void Museek::SearchManager::buddySearch(uint token, const std::string & query) {
    std::vector<std::string> buddies = museekd()->config()->keys("buddies");
    std::vector<std::string>::const_iterator it;
    std::string q = museekd()->codeset()->toNet(query);
    for(it = buddies.begin(); it != buddies.end(); ++it) {
        SUserSearch msg(*it, token, q);
        museekd()->server()->sendMessage(msg.make_network_packet());
    }
}

/**
  * Initiate a search in the room we joined
  */
void Museek::SearchManager::roomsSearch(uint token, const std::string & query) {
    std::vector<std::string> rooms = museekd()->server()->joinedRooms();
    std::vector<std::string>::const_iterator it;
    std::string q = museekd()->codeset()->toNet(query);
    for(it = rooms.begin(); it != rooms.end(); ++it) {
        SRoomSearch msg(*it, token, q);
        museekd()->server()->sendMessage(msg.make_network_packet());
    }
}

/**
  * Add an item to wishlist and search for it
  */
void Museek::SearchManager::wishlistAdd(const std::string & query) {
    uint token = museekd()->token();
    museekd()->ifaces()->sendNewSearchToAll(query, token);

    if (m_Wishlist.find(query) != m_Wishlist.end()) {
        NNLOG("museekd.peers.debug", "'%s' is already in the wishlist", query.c_str());
        // This item is already in wishlist, just do a normal search
        SFileSearch msg(token, museekd()->codeset()->toNet(query));
        museekd()->server()->sendMessage(msg.make_network_packet());
    }
    else {
        NNLOG("museekd.peers.debug", "Adding '%s' in the wishlist", query.c_str());
        // launch a wishlist search
        SWishlistSearch msg(token, museekd()->codeset()->toNet(query));
        museekd()->server()->sendMessage(msg.make_network_packet());
    }
    // Update item's last query date
    museekd()->config()->set("wishlist", query, static_cast<uint>(time(NULL)));
}

/**
  * There's a peer socket available: see if we have something to send
  */
void Museek::SearchManager::onPeerSocketReady(PeerSocket * socket) {
    std::string username = socket->user();

    std::map<std::string, std::map<uint, Folder> >::iterator pending = m_PendingResults.find(username);
    if (pending != m_PendingResults.end() && m_PendingResults[username].size()) {
        NNLOG("museekd.peers.debug", "Sending search results to %s", username.c_str());

        std::map<uint, Folder>::const_iterator it;
        for (it = m_PendingResults[username].begin(); it != m_PendingResults[username].end(); it++) {
            PSearchReply msg(it->first, username, it->second, transferSpeed(), (uint64) museekd()->uploads()->queueTotalLength(), museekd()->uploads()->hasFreeSlots());
            socket->sendMessage(msg.make_network_packet());
        }

        m_PendingResults.erase(pending);

        // Disconnect the peer socket as it is probably no longer needed and we have a limit for opened socket
        socket->addSearchResultsOnlyTimeout(2000);
    }
}

/**
  * Our connection with our parent has been disconnected
  */
void Museek::SearchManager::onParentDisconnected(NewNet::ClientSocket * socket_) {
    m_BranchRoot = museekd()->server()->username();
    setBranchLevel(0);

    setParent(0);

    SAcceptChildren msgAccept(false);
    museekd()->server()->sendMessage(msgAccept.make_network_packet());

    SChildDepth msgD(childDepth());
    museekd()->server()->sendMessage(msgD.make_network_packet());

    SHaveNoParents msgH(true);
    museekd()->server()->sendMessage(msgH.make_network_packet());

    SBranchLevel msgL(branchLevel());
    museekd()->server()->sendMessage(msgL.make_network_packet());

    SBranchRoot msgR(branchRoot());
    museekd()->server()->sendMessage(msgR.make_network_packet());
}

/**
  * Our connection with our child has been disconnected
  */
void Museek::SearchManager::onChildDisconnected(NewNet::ClientSocket * socket_) {
    DistributedSocket * socket = (DistributedSocket *) socket_;

    NNLOG("museekd.peers.debug", "Child %s is gone", socket->user().c_str());

    removeChild(socket->user());
}

/*
    Called when the connection to the server changes (connected/disconnected)
*/
void
Museek::SearchManager::onServerLoggedInStateChanged(bool loggedIn)
{
    if(loggedIn) {
        uint level = branchLevel();
        uint depth = branchLevel();
        DistributedSocket * parentSocket = parent();
        std::string parentName = museekd()->server()->username();
        if (parentSocket)
            parentName = parentSocket->user();

        NNLOG("museekd.peers.debug", "Asking parents (level: %d, root: %s, child depth:%d)", level, parentName.c_str(), depth);

        SHaveNoParents msgNoParents(true);
        museekd()->server()->sendMessage(msgNoParents.make_network_packet());

        SBranchLevel msgLevel(level);
        museekd()->server()->sendMessage(msgLevel.make_network_packet());

        SBranchRoot msgRoot(parentName);
        museekd()->server()->sendMessage(msgRoot.make_network_packet());

        SChildDepth msgDepth(depth);
        museekd()->server()->sendMessage(msgDepth.make_network_packet());

        SAcceptChildren msgAccept(true);
        museekd()->server()->sendMessage(msgAccept.make_network_packet());

        // We want to know our own transfer speed. Ask the server for it.
        museekd()->peers()->requestUserData(museekd()->server()->username());
    }
    else if(!parent()) {
        // If we're the root of our branch and we get disconnected, we should disconnect from our children to let them go on a living branch
        std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, uint> >::iterator it;
        for (it = m_Children.begin(); it != m_Children.end(); it++) {
            if (it->second.first)
                it->second.first->stop();
        }
        m_Children.clear();
    }
}

/*
    Called when the connection cannot be made with the peer.
*/
void
Museek::SearchManager::onPeerSocketUnavailable(std::string user)
{
    // Could not connect to the peer or disconnected: delete the pending search results
    std::map<std::string, std::map<uint, Folder> >::iterator it;
    it = m_PendingResults.find(user);
    if (it != m_PendingResults.end())
        m_PendingResults.erase(it);
}

void
Museek::SearchManager::searchReplyReceived(uint ticket, const std::string & user, bool slotfree, uint avgspeed, uint64 queuelen, const Folder & folders) {
    museekd()->ifaces()->onSearchReply(ticket, user, slotfree, avgspeed, (uint) queuelen, folders);
}

void
Museek::SearchManager::onConfigKeySet(const ConfigManager::ChangeNotify * data) {
    if ((data->domain == "wishlist") && (!data->key.empty()))
        m_Wishlist[data->key] = museekd()->config()->getInt(data->domain, data->key);
}

void
Museek::SearchManager::onConfigKeyRemoved(const ConfigManager::RemoveNotify * data) {
    if (data->domain == "wishlist") {
        std::map<std::string, time_t>::iterator it = m_Wishlist.find(data->key);
        if (it != m_Wishlist.end())
            m_Wishlist.erase(it);
    }
}

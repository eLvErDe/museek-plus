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
#ifndef MUSEEK_SEARCHMANAGER_H
#define MUSEEK_SEARCHMANAGER_H

#include <NewNet/nnobject.h>
#include <NewNet/nnweakrefptr.h>
#include <NewNet/nnrefptr.h>
#include <NewNet/nnevent.h>
#include <NewNet/nnbuffer.h>
#include "mutypes.h"
#include "servermessages.h"
#include "peermessages.h"
#include "distributedsocket.h"
#include "configmanager.h"

/* Forward declarations. */
class SGetStatus;

namespace NewNet
{
  class ClientSocket;
}

namespace Museek
{
  class Museekd;
  class PeerSocket;

  /* The search manager manages .. searches. */
  class SearchManager : public NewNet::Object
  {
  public:
    SearchManager(Museekd * museekd);
    ~SearchManager();

    /* Return pointer to museekd instance. */
    Museekd * museekd() const { return m_Museekd; }

    uint childDepth();

    void setChild(DistributedSocket * socket, uint depth);
    void removeChild(const std::string & user);

    void addPotentialParent(const std::string & user, DistributedSocket * socket, const std::string & ip);
    bool isPotentialParent(const std::string & user) {return m_PotentialParents.find(user) != m_PotentialParents.end();};

    DistributedSocket * parent() { return m_Parent; };
    void setParent(DistributedSocket * parent);

    uint branchLevel() {return m_BranchLevel;};
    void setBranchLevel(uint level) {m_BranchLevel = level;};

    std::string branchRoot() {return m_BranchRoot;};
    void setBranchRoot(const std::string & root);

    uint transferSpeed() {return m_TransferSpeed;};
    void setTransferSpeed(uint speed) {m_TransferSpeed = speed;};

    void searchReplyReceived(uint ticket, const std::string & user, bool slotfree, uint avgspeed, uint64 queuelen, const Folder & folders);
    void branchLevelReceived(DistributedSocket * socket, uint level);

    void transmitSearch(uint unknown, const std::string & username, uint ticket, const std::string & query);
    void sendSearchResults(const std::string & username, const std::string & query, uint token);

    bool acceptChildren() {return m_Children.size() < m_ChildrenMaxNumber;};

    void buddySearch(uint token, const std::string & query);
    void roomsSearch(uint token, const std::string & query);
    void wishlistAdd(const std::string & query);

  protected:
    /* Find or make a peer socket for the specified user. */
    PeerSocket * peerSocket(const std::string & user);

  private:
    void onServerLoggedInStateChanged(bool loggedIn);
    void onPeerSocketUnavailable(std::string user);
    void onNetInfoReceived(const SNetInfo * msg);
    void onSearchRequested(const SSearchRequest * msg);
    void onFileSearchRequested(const SFileSearch * msg);
    void onAddUserReceived(const SAddUser * msg);
    void onWishlistIntervalReceived(const SWishlistInterval * msg);
    void onPeerSocketReady(PeerSocket * socket);
    void onParentDisconnected(NewNet::ClientSocket * socket_);
    void onChildDisconnected(NewNet::ClientSocket * socket_);
    void onConfigKeySet(const ConfigManager::ChangeNotify * data);
    void onConfigKeyRemoved(const ConfigManager::RemoveNotify * data);
    void onWishlistTimeout(long);

    NewNet::WeakRefPtr<Museekd>                 m_Museekd;          // Ref to the museekd
    std::string                                 m_ParentIp;         // The IP address of our parent
    std::string                                 m_BranchRoot;       // Parent of the branch we're in
    uint                                        m_BranchLevel;      // Position in the branch we're in (starting from top)
    uint                                        m_TransferSpeed;    // Our own transfer speed
    NewNet::RefPtr<DistributedSocket>           m_Parent;           // Parent's socket
    uint                                        m_ChildrenMaxNumber;// Maximum number of child allowed
    uint                                        m_WishlistInterval; // Wishlist interval
    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, std::string> >
                                                m_PotentialParents; // Potential parent we're connecting to
    std::map<std::string, std::pair<NewNet::RefPtr<DistributedSocket>, uint> >
                                                m_Children;         // List of all our children with their respective depth
    std::map<std::string, std::map<uint, Folder> >
                                                m_PendingResults;   // Pending search results we'll have to send soon
    std::map<std::string, time_t>               m_Wishlist;         // Wishlist items with the last time we searched for them
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_WishlistTimeout; // Wishlist timeout
  };
}

#endif // MUSEEK_SEARCHMANAGER_H

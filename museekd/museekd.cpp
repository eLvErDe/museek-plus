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
#include "museekd.h"
#include "configmanager.h"
#include "codesetmanager.h"
#include "servermanager.h"
#include "peermanager.h"
#include "ifacemanager.h"
#include "peersocket.h"
#include "downloadmanager.h"
#include "uploadmanager.h"
#include "sharesdatabase.h"
#include "searchmanager.h"
#include <NewNet/nnreactor.h>
#include <fstream>

Museek::Museekd::Museekd(NewNet::Reactor * reactor) : m_Reactor(reactor)
{
  /* Seed the random generator and fabricate our starting token. */
  srand(time(NULL));
  m_Token = rand();

  /* Did we get a reactor? No? Create one. */
  if(! reactor)
  {
    m_Reactor = new NewNet::Reactor();
  }

  /* Instantiate the various components. Order can be important here. */
  m_Config = new ConfigManager();
  m_Codeset = new CodesetManager(this);
  m_Server = new ServerManager(this);
  m_Peers = new PeerManager(this);
  m_Downloads = new DownloadManager(this);
  m_Uploads = new UploadManager(this);
  m_Ifaces = new IfaceManager(this);
  m_Shares = new SharesDatabase(this);
  m_BuddyShares = new SharesDatabase(this);
  m_Searches = new SearchManager(this);
}

void Museek::Museekd::LoadShares() {
  std::string shares = m_Config->get("shares", "database");
  if (! shares.empty()) {
    m_Shares->load(shares);
    m_BuddyShares->load(shares);
  }
  std::string bshares = m_Config->get("buddy.shares", "database");
  if (! bshares.empty() && haveBuddyShares())
    m_BuddyShares->load(bshares, (!shares.empty()));
}

void Museek::Museekd::LoadDownloads() {
    m_Downloads->loadDownloads();
}

Museek::Museekd::~Museekd()
{
  /* This destructor doesn't do much. */
  NNLOG("museekd.debug", "museekd destroyed");
}

bool Museek::Museekd::isBanned(const std::string u) {
    return config()->hasKey("banned", u);
}

bool Museek::Museekd::isIgnored(const std::string u) {
    return config()->hasKey("ignored", u);
}

bool Museek::Museekd::isTrusted(const std::string u) {
    return config()->hasKey("trusted", u);
}

bool Museek::Museekd::isBuddied(const std::string u) {
    return config()->hasKey("buddies", u);
}

bool Museek::Museekd::isPrivileged(const std::string u) {
    return std::find(mPrivilegedUsers.begin(), mPrivilegedUsers.end(), u) != mPrivilegedUsers.end();
}

bool Museek::Museekd::toBuddiesOnly() {
    return config()->getBool("transfers", "only_buddies", false);
}

bool Museek::Museekd::haveBuddyShares() {
    return config()->getBool("transfers", "have_buddy_shares", false);
}

bool Museek::Museekd::trustingUploads() {
    return config()->getBool("transfers", "trusting_uploads", false);
}

bool Museek::Museekd::autoClearFinishedDownloads() {
    return config()->getBool("transfers", "autoclear_finished_downloads", false);
}

bool Museek::Museekd::autoRetryDownloads() {
    return config()->getBool("transfers", "autoretry_downloads", false);
}

bool Museek::Museekd::autoClearFinishedUploads() {
    return config()->getBool("transfers", "autoclear_finished_uploads", false);
}

bool Museek::Museekd::privilegeBuddies() {
    return config()->getBool("transfers", "privilege_buddies", false);
}

uint Museek::Museekd::upSlots() {
    return config()->getUint("transfers", "upload_slots", 0);
}

uint Museek::Museekd::downSlots() {
    return config()->getUint("transfers", "download_slots", 0);
}

// Add this user to the list of privileged ones
void Museek::Museekd::addPrivilegedUser(const std::string & user) {
    if (!isPrivileged(user)) {
        mPrivilegedUsers.push_back(user);
        NNLOG("museekd.debug", "%u privileged users", mPrivilegedUsers.size());
    }
}

// Replace the privileged users list with this new one
void Museek::Museekd::setPrivilegedUsers(const std::vector<std::string> & users) {
    mPrivilegedUsers = users;
    NNLOG("museekd.debug", "%u privileged users", mPrivilegedUsers.size());
}

void Museek::Museekd::sendSharedNumber() {
    uint32 numFiles = m_Shares->files();
    uint32 numFolders = m_Shares->folders();
    if (numFiles == 0) {
        numFiles = m_BuddyShares->files();
        numFolders = m_BuddyShares->folders();
    }

	if  (server()->loggedIn()) {
	    SSharedFoldersFiles msg(numFolders, numFiles);
	    server()->sendMessage(msg.make_network_packet());
	}
}

bool Museek::Museekd::isEnabledPrivRoom() {
    return config()->getBool("priv_rooms", "enable_priv_room", false);
}

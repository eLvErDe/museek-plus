/* Museek - Museek's 'core' library
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Museek/mu.hh>

#include <Museek/PeerManager.hh>
#include <Museek/Museek.hh>
#include <Museek/PeerConnection.hh>
#include <Museek/TransferManager.hh>
#include <Museek/TransferConnection.hh>

#define MULOG_DOMAIN "Museek.PM"
#include <Muhelp/Mulog.hh>

#include <string>
#include <map>
#include <queue>
#include <vector>

using std::string;
using std::wstring;
using std::map;
using std::queue;
using std::vector;
using std::pair;

#define ALL_PEERS(code) \
	{ map<string, Peer*>::iterator p_it = mPeers.begin(); \
	  for(; p_it != mPeers.end(); ++p_it) { \
	  (*p_it).second->code; } \
	}

PeerManager::PeerManager(Museek* museek) : mMuseek(museek) {
	CT("PeerManager");
}

void PeerManager::server_connected() {
	CT("server_connected");
	
	ALL_PEERS(server_connected());
	get_peer(mMuseek->username())->subscribe();
}

void PeerManager::server_disconnected() {
	CT("server_connected");
	
	ALL_PEERS(server_disconnected());
}

Peer* PeerManager::get_peer(const string& user, bool can_make) {
	CT("get_peer %s, %d", user.c_str(), can_make);
	
	map<string, Peer*>::iterator it = mPeers.find(user);
	if(it == mPeers.end()) {
		if(can_make) {
			mPeers[user] = new Peer(this, user);
			return mPeers[user];
		} else {
			return 0;
		}
	} else {
		return (*it).second;
	}
}

void PeerManager::remove_peer(Peer* peer) {
	CT("remove_peer %s", peer->user().c_str());
	
	mPeers.erase(peer->user());
}

void PeerManager::purge() {
	CT("purge");
	
	map<string, Peer*>::iterator it = mPeers.begin();
	queue<Peer*> peers;
	
	for(; it != mPeers.end(); ++it)
		if((*it).second->ref_count() == 0 && (*it).first != mMuseek->username())
			peers.push((*it).second);
		else
			(*it).second->flush_downloads();
	
	while(! peers.empty()) {
		delete peers.front();
		peers.pop();
	}
}

void PeerManager::set_peer_exists(const string& user, bool exists) {
	CT("set_peer_exists %s, %d", user.c_str(), exists);
	
	if(Peer* peer = get_peer(user, false))
		peer->set_exists(exists);
}

void PeerManager::set_peer_status(const string& user, uint32 status) {
	CT("set_peer_status %s, %u", user.c_str(), status);
	
	Peer* peer;
// 	if (status == 0) {
// 		if((peer = get_peer(user, true))) {
// 			remove_peer(peer);
// 		}
// 	} else {
		if((peer = get_peer(user, false)))
			peer->set_status(status);
// 	}
}

void PeerManager::set_peer_address(const string& user, const string& ip, uint32 port) {
	CT("set_peer_address %s, %s, %u", user.c_str(), ip.c_str(), port);
	
	if(Peer* peer = get_peer(user, false)) {
		peer->set_address(ip, port); 
		}
}

void PeerManager::set_peer_stats(const string& user, uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs) {
	CT("set_peer_stats %s, %u, %u, %u, %u", user.c_str(), avgspeed, downloadnum, files, dirs);
	
	if(Peer* peer = get_peer(user, false))
		peer->set_stats(avgspeed, downloadnum, files, dirs);
}

void PeerManager::request_peer_connect(const string& user, const string& type, const string& ip, uint32 port, uint32 token) {
	CT("request_peer_connect %s, %s, %u, %u", user.c_str(), ip.c_str(), port, token);
	
	Peer* peer = get_peer(user);
	peer->set_address(ip, port);
	peer->request_connect(type, token);
}

void PeerManager::peer_cannot_connect(const string& user, uint32 token) {
	CT("peer_cannot_connect %s, %u", user.c_str(), token);
	
	if(user.empty()) {
		map<string, Peer*>::iterator it = mPeers.begin();
		for(; it != mPeers.end(); ++it)
			(*it).second->cannot_connect(token);
	} else {
		if(Peer* peer = get_peer(user, false))
			peer->cannot_connect(token);
	}
}

bool PeerManager::pierced_firewall(int sock, uint32 token) {
	CT("pierced_firewall %i, %u", sock, token);
	
	map<string, Peer*>::iterator it = mPeers.begin();
	for(; it != mPeers.end(); ++it)
		if((*it).second->pierced_firewall(sock, token))
			return true;
	
	return false;
}

#undef MULOG_DOMAIN
#define MULOG_DOMAIN "Museek.PE"

Peer::Peer(PeerManager* manager, const string& user)
     : mManager(manager), mRefCount(0), mUser(user),
       mSubscribed(false), mHaveExists(false), mHaveStatus(false), mHaveAddress(false), mHaveStats(false),
       mPort(0), mStatus(0), mAvgSpeed(0), mDownloadNum(0), mFiles(0), mDirs(0), mUploading(false) {
	CT("Peer %s", user.c_str());
	mPrivileged = mManager->museek()->is_privileged(user);
	mAllowUploads = mManager->museek()->is_trusted(user);
}

Peer::~Peer() {
	CT("~Peer (%s)", mUser.c_str());
	
	DEBUG("gurgle gurgle... %s is removed from the list of peers", mUser.c_str());
	mManager->remove_peer(this);
}

void Peer::server_connected() {
	CT("server_connected");
	
	if(mSubscribed) {
		mSubscribed = false;
		subscribe();
	}
}

void Peer::server_disconnected() {
	CT("server_disconnected");
	
	set_exists(false);
	set_status(0);
	set_address("0.0.0.0", 0);
	set_stats(0, 0, 0, 0);
	
	mHaveAddress = mHaveStatus = mHaveExists = mHaveStats = false;
}

void Peer::inc_ref() {
	CT("inc_ref (%s)", mUser.c_str());
	
	++mRefCount;
}

void Peer::dec_ref(void* connection) {
	CT("dec_ref (%s)", mUser.c_str());
	
	vector<PeerConnection*>::iterator it = find(mConnections.begin(), mConnections.end(), (PeerConnection*)connection);
	if(it != mConnections.end())
		mConnections.erase(it);
	
	if(mRefCount == 0) {
		DEBUG("WARNING: RefCount underflow!");
	} else {
		mRefCount--;
	}
}

void Peer::add_transfer(Transfer* transfer) {
	CT("add_transfer %s (%s)", transfer->path().c_str(), mUser.c_str());
	
	inc_ref();
	mTransfers.push_back(transfer);
	
	if(! mSubscribed) {
		subscribe();
	} else if(mHaveStatus) {
		transfer->set_status(mStatus);
	}
}

void Peer::remove_transfer(Transfer* transfer) {
	CT("remove_transfer %s (%s)", transfer->path().c_str(), mUser.c_str());
	
	vector<Transfer*>::iterator it = find(mTransfers.begin(), mTransfers.end(), transfer);
	if(it != mTransfers.end()) {
		mTransfers.erase(it);
		dec_ref();
	} else {
		DEBUG("couldn't find transfer!");
	}
	if(mUploading == transfer) {
		mUploading = 0;
		mManager->museek()->transfer_manager()->check_uploads();
	}
}

void Peer::subscribe() {
	CT("subscribe (%s)", mUser.c_str());
	
	if(! mSubscribed) {
		mSubscribed = true;
		mManager->museek()->server_peer_subscribe(mUser);
		mManager->museek()->server_get_peer_status(mUser);
		mManager->museek()->server_get_peer_stats(mUser);
		mManager->museek()->server_get_peer_address(mUser);
	}
}

void Peer::set_exists(bool exists) {
	CT("set_exists %d (%s)", exists, mUser.c_str());
	
	mHaveExists = mSubscribed;
	mExists = exists;
}

void Peer::set_status(uint32 status) {
	CT("set_status %u (%s)", status, mUser.c_str());
	
	mStatus = status;
	mHaveStatus = mSubscribed;
	
	vector<PeerConnection*>::iterator it, end = mConnections.end();
	for(it = mConnections.begin(); it != end; ++it)
	{
		if(status == 0 && (*it)->state() != BS_Connected)
			(*it)->disconnect();
		
		if((*it)->state() == BS_Status)
		{
			if(mHaveAddress)
				(*it)->connect();
			else {
				(*it)->set_state(BS_Address);
				mManager->museek()->server_get_peer_address(mUser);
			}
		}
	}
	
	vector<Transfer*>::iterator trit = mTransfers.begin();
	for(; trit != mTransfers.end(); ++trit)
		(*trit)->set_status(status);
	
	flush_downloads();
}

void Peer::push_download(uint ticket, const wstring& path) {
	mDownloadQueue.push_back(pair<uint, wstring>(ticket, path));
}

void Peer::flush_downloads() {
	if(mDownloadQueue.empty())
		return;
	
	PeerConnection *mConnection = connection();
	
	vector<pair<uint, wstring> >::iterator it, end = mDownloadQueue.end();
	for(it = mDownloadQueue.begin(); it != end; ++it)
		mConnection->download((*it).first, (*it).second);
	
	mDownloadQueue.clear();
	
}

void Peer::set_address(const string& ip, uint32 port) {
	CT("set_address %s, %u (%s)", ip.c_str(), port, mUser.c_str());
	
	if(ip == "0.0.0.0") {
		set_status(0);
		return;
	}
	
// 	if(port == 0) {
// 		mManager->museek()->server_get_peer_address(mUser);
// 		return;
// 	}
	if ( mUser == mManager->museek()->username() ){
			DEBUG("connecting to yourself");
			mIP = "127.0.0.1";
		}
	else {
		mIP = ip; }
	mPort = port;
	mHaveAddress = mSubscribed;
	
	vector<PeerConnection*>::iterator pit, pend = mConnections.end();
	for(pit = mConnections.begin(); pit != pend; ++pit)
		if((*pit)->state() == BS_Address)
			(*pit)->connect();
	
	vector<Transfer*>::iterator it = mTransfers.begin();
	for(; it != mTransfers.end(); ++it) {
		BaseConnection* conn = (*it)->connection();
		if(conn && conn->state() == BS_Address)
			conn->connect();
	}
}

void Peer::set_stats(uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs) {
	DEBUG("set_stats %u, %u, %u, %u (%s)", avgspeed, downloadnum, files, dirs, mUser.c_str());
	
	mAvgSpeed = avgspeed;
	mDownloadNum = downloadnum;
	mFiles = files;
	mDirs = dirs;
	mHaveStats = mSubscribed;
}

void Peer::request_connect(const string& type, uint32 token) {
	CT("request_connect %u (%s)", token, mUser.c_str());
	
	if(type == "P") {
		PeerConnection *mConnection = new PeerConnection(this, token);
		mConnections.push_back(mConnection);
		mConnection->request_connect();
	} else if(type == "F") {
		(new TransferPreConnection(this, token))->request_connect();
	}
}

void Peer::cannot_connect(uint32 token) {
	CT("cannot_connect %u (%s)", token, mUser.c_str());
	
	bool matched = false;
	
	vector<PeerConnection*>::iterator it, end = mConnections.end();
	for(it = mConnections.begin(); it != end; ++it)
	{
		if((*it)->state() == BS_Waiting && (*it)->token() == token) {
			(*it)->disconnect();
			matched = true;
		}
	}
	
	if(matched)
	{
			vector<Transfer*>::iterator tr_it = mTransfers.begin();
			for(; tr_it != mTransfers.end(); ++tr_it)
				if((*tr_it)->state() == TS_Connecting || (*tr_it)->state() == TS_Initiating)
					(*tr_it)->set_state(TS_CannotConnect);
	}
}

PeerConnection* Peer::connection() {
	CT("connection (%s)", mUser.c_str());
	
	// Try to find an idle connection
	vector<PeerConnection*>::iterator it, end = mConnections.end();
	for(it = mConnections.begin(); it != end; ++it)
		if((*it)->get_error() == ERR_NONE && ! (*it)->locked())
			return *it;
	
	// Spawn new connection and send init message
	PeerConnection *mConnection = new PeerConnection(this, mManager->museek()->token());
	mConnections.push_back(mConnection);

	if(mManager->museek()->connect_mode() == CM_Passive) {
		mConnection->pierce_firewall();
	} else {
		mConnection->init();
		
		if(! mHaveStatus) {
			mConnection->set_state(BS_Status);
			mManager->museek()->server_get_peer_status(mUser);
		} else if(! mHaveAddress) {
			mConnection->set_state(BS_Address);
			mManager->museek()->server_get_peer_address(mUser);
		} else
			mConnection->connect();
	}
	
	return mConnection;
}

void Peer::set_socket(int sock, uint32 token, const queue<unsigned char>& data) {
	CT("set_socket %i, %u (%s)", sock, token, mUser.c_str());
	
	PeerConnection *mConnection = new PeerConnection(this, token);
	mConnections.push_back(mConnection);
	mConnection->set_socket(sock, data);
}

bool Peer::pierced_firewall(int sock, uint32 token) {
	CT("pierced_firewall %i, %u (%s)", sock, token, mUser.c_str());
	
	vector<PeerConnection*>::iterator pit, pend = mConnections.end();
	for(pit = mConnections.begin(); pit != pend; ++pit)
	{
		if((*pit)->state() == BS_Waiting && (*pit)->token() == token) {
			(*pit)->pierced_firewall(sock);
			return true;
		}
	}
	
	vector<Transfer*>::const_iterator it = mTransfers.begin();
	for(; it != mTransfers.end(); ++it) {
		BaseConnection *conn = (*it)->connection();
		if(conn && conn->state() == BS_Waiting && conn->token() == token) {
			conn->pierced_firewall(sock);
			return true;
		}
	}
	
	return false;
}

Transfer* Peer::download(const wstring& path) {
	vector<Transfer*>::const_iterator it = mTransfers.begin();
	for(; it != mTransfers.end(); ++it)
		if((*it)->direction() == Transfer::Download && (*it)->path() == path)
			return *it;
	return 0;
}

Transfer* Peer::download(uint32 ticket) {
	vector<Transfer*>::const_iterator it = mTransfers.begin();
	for(; it != mTransfers.end(); ++it)
		if((*it)->direction() == Transfer::Download && (*it)->ticket_valid() && (*it)->ticket() == ticket)
			return *it;
	return 0;
}

void Peer::abort_download(const wstring& path) {
	Transfer* transfer = download(path);
	if(transfer)
		transfer->set_state(TS_Aborted);
}

Transfer* Peer::upload(const wstring& path) {
	vector<Transfer*>::const_iterator it = mTransfers.begin();
	for(; it != mTransfers.end(); ++it)
		if((*it)->direction() == Transfer::Upload && (*it)->path() == path)
			return *it;
	return 0;
}

Transfer* Peer::upload(uint32 ticket) {
	vector<Transfer*>::const_iterator it = mTransfers.begin();
	for(; it != mTransfers.end(); ++it)
		if((*it)->direction() == Transfer::Upload && (*it)->ticket_valid() && (*it)->ticket() == ticket)
			return *it;
	return 0;
}

void Peer::abort_upload(const wstring& path) {
	Transfer* transfer = upload(path);
	if(transfer)
		transfer->set_state(TS_Aborted);
}

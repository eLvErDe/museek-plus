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

#include <Museek/Museek.hh>
#include <Museek/Recoder.hh>
#include <Museek/ServerConnection.hh>
#include <Museek/PeerManager.hh>
#include <Museek/PeerListener.hh>
#include <Museek/ShareBase.hh>
#include <Museek/PeerConnection.hh>
#include <Museek/DistribManager.hh>
#include <Museek/TransferManager.hh>
#include <Museek/TransferConnection.hh>

#define MULOG_DOMAIN "Museek.MU"
#include <Muhelp/Mulog.hh>

#include <string>
#include <queue>
#include <vector>

using std::string;
using std::wstring;
using std::queue;
using std::vector;
using std::map;

Museek::Museek() : ConnectionManager(), mListenPort(0), mUserWarnings(false), mBuddySharesHave(true),  mBuddiesOnly(false), mRecoder(0), 
                   mServerHost(""), mServerPort(0), mUsername(""), mPassword(""),
                   mServer(0), mConnected(false), mLoggedIn(false),
                   mParentInactivityTimeout(0), mSearchInactivityTimeout(0), mMinParentsInCache(0),
                   mDistribAliveInterval(0), mWishListInterval(0), 
                   mShares(0),  mBuddyShares(0), mConnectMode(CM_Active),  mPeerManager(0),
                   mUploadSlots(1),

                   mTransferManager(0),
                   mPeerListener(0), mMinPort(2234), mMaxPort(2240) {
	
	srandom(time(NULL));
	mToken = random();
}

void Museek::init() {
	init_recoder();
	init_shares();
	init_buddies_shares();
	init_peer_manager();
	init_distrib_manager();
	init_transfer_manager();
	init_peer_listener();
}

void Museek::init_recoder() {
	mRecoder = new Recoder("ISO8859-1", "UTF-8");
}

void Museek::init_shares() {
	mShares = new ShareBase(this);
}
void Museek::init_buddies_shares() {
	mBuddyShares = new ShareBase(this);
}
void Museek::init_peer_manager() {
	mPeerManager = new PeerManager(this);
}

void Museek::init_distrib_manager() {
	mDistribManager = new DistribManager(this);
}

void Museek::init_transfer_manager() {
	mTransferManager = new TransferManager(this);
}

void Museek::init_peer_listener() {
	mListenPort = 0;
	if (mMinPort == mMaxPort) {
		mPeerListener = new PeerListener(this);
		mPeerListener->listen("", mMinPort);
		if(mPeerListener->get_error() == ERR_NONE) {
			mListenPort = mMinPort;
		} else {
			mPeerListener = 0;
		}
	} else if (mMinPort > mMaxPort) {
		for(uint i = mMaxPort; i < mMinPort; ++i) {
			mPeerListener = new PeerListener(this);
			mPeerListener->listen("", i);
			if(mPeerListener->get_error() == ERR_NONE) {
				mListenPort = i;
				break;
			}
			
			mPeerListener = 0;
		}
	} else {
		for(uint i = mMinPort; i < mMaxPort; ++i) {
			mPeerListener = new PeerListener(this);
			mPeerListener->listen("", i);
			if(mPeerListener->get_error() == ERR_NONE) {
				mListenPort = i;
				break;
			}
			
			mPeerListener = 0;
		}
	}
}

void Museek::cycle_callback() {
	if(mPeerManager)
		mPeerManager->purge();
	if(mDistribManager)
		mDistribManager->cycle();
}

bool Museek::is_privileged(const string& user) {
	map<string, bool>::const_iterator it = mPrivileged.find(user);
	if(it != mPrivileged.end())
		return (*it).second;
	
	server_get_peer_privileged(user);
	return false;
}
void Museek::add_receiving(const string& user){
}
void Museek::remove_receiving(const string& user){
}
bool Museek::is_receiving_shares(const string& user) const {
	return false;
}
bool Museek::is_banned(const string& user) const {
	return false;
}
bool Museek::is_buddied(const string& user) const {
	return false;
}

bool Museek::is_trusted(const string& user) const {
	map<string, bool>::const_iterator it = mAllowUploads.find(user);
	if(it != mAllowUploads.end()) {
		return (*it).second; }
	return false;
}
void Museek::mu_set_user_warnings(bool uw) {
	mUserWarnings = uw;
}
void Museek::mu_set_only_buddies(bool only) {
	mBuddiesOnly = only;
}
void Museek::server_connect(uint timeout) {
	CT("server_connect");
	
	if(mServer)
		server_disconnect();
	
	mServer = new ServerConnection(this);
	mServer->connect(mServerHost, mServerPort, timeout);
	if(mServer)
		add(mServer);
}

void Museek::server_disconnect() {
	CT("server_disconnect");
	
	if(mServer)
		mServer->disconnect();
}

void Museek::server_log_in(const string& username, const string& password) {
	CT("server_log_in");
	
	if(mServer) {
		mServer->log_in(username, password);
		mServer->listen_port(mListenPort);
	}
}

void Museek::server_shared_folders_files(uint32 folders, uint32 files) {
	if(mServer)
		mServer->shared_folders_files(folders, files);
		mServer->get_peer_stats(username());
}

void Museek::server_set_status(uint32 s) {
	if(mServer)
		mServer->set_status(s);
}

void Museek::server_check_privileges() {
	if(mServer)
		mServer->check_privileges();
}

void Museek::server_peer_subscribe(const string& user) {
	if(mServer)
		mServer->peer_subscribe(user);
}

void Museek::server_get_peer_address(const string& user) {
	if(mServer)
		mServer->get_peer_address(user);
}

void Museek::server_get_peer_status(const string& user) {
	if(mServer)
		mServer->get_peer_status(user);
}

void Museek::server_get_peer_stats(const string& user) {
	if(mServer)
		mServer->get_peer_stats(user);
}

void Museek::server_get_peer_privileged(const string& user) {
	if(mServer)
		mServer->get_peer_privileged(user);
}

void Museek::server_request_peer_connect(uint32 token, const string& user, const string& type) {
	if(mServer)
		mServer->request_peer_connect(token, user, type);
}

void Museek::server_peer_cannot_connect(uint32 token, const string& user) {
	if(mServer)
		mServer->peer_cannot_connect(token, user);
}

void Museek::server_join_room(const string& room) {
	if(mServer)
		mServer->join_room(room);
}

void Museek::server_leave_room(const string& room) {
	if(mServer)
		mServer->leave_room(room);
}

void Museek::server_say_room(const string& room, const wstring& message) {
	if(mServer)
		mServer->say_room(room, message);
}

void Museek::server_set_ticker(const string& room, const wstring& message) {
	if(mServer)
		mServer->set_ticker(room, message);
}

void Museek::server_room_list() {
	if(mServer)
		mServer->room_list();
}
void Museek::server_send_private(const string& user, const wstring& message) {
	if(mServer)
		mServer->send_private(user, message);
}

void Museek::server_ack_private(uint32 msg_id) {
	if(mServer)
		mServer->ack_private(msg_id);
}

void Museek::server_search(uint32 ticket, const wstring& query) {
	if(mServer)
		mServer->search(ticket, query);
}
void Museek::server_room_search(const string& room, uint32 ticket, const wstring& query ) {
	if(mServer)
		mServer->room_search( room,  ticket, query);
}
void Museek::server_user_search(const string& user, uint32 ticket, const wstring& query) {
	if(mServer)
		mServer->user_search(user, ticket, query);
}
void Museek::server_wishlist_search(uint32 ticket, const wstring& query) {
	if(mServer)
		mServer->wishlist_search(ticket, query);
}
void Museek::server_no_parent(bool have_parent) {
	if(mServer)
		mServer->no_parent(have_parent);
}

void Museek::server_give_privileges(const string& user, uint32 days) {
	if(mServer)
		mServer->give_privileges(user, days);
}

void Museek::cb_server_cannot_resolve() {
	mServer = NULL;
}

void Museek::cb_server_cannot_connect() {
	mServer = NULL;
}

void Museek::cb_server_cannot_login() {
	/* don't do anything here, ServerConnection will disconnect and call cb_server_disconnected() */
}

void Museek::cb_server_disconnected() {
	mServer = NULL;
	mConnected = mLoggedIn = false;
	
	mPeerManager->server_disconnected();
	mDistribManager->server_disconnected();
}

void Museek::cb_server_kicked() {
	mServer = NULL;
	mConnected = mLoggedIn = false;
	
	mPeerManager->server_disconnected();
	mDistribManager->server_disconnected();
}

void Museek::cb_server_connected() {
	mConnected = true;
	server_log_in(username(), password());
}

void Museek::cb_server_logged_in(bool success, const string& message) {
	DEBUG("Logged in: %d\n%s", success, message.c_str());
	mLoggedIn = success;
	if(success) {
		if (mBuddiesOnly && mBuddySharesHave)
			server_shared_folders_files(mBuddyShares->folders(), mBuddyShares->files());
		else {
			server_shared_folders_files(mShares->folders(), mShares->files());
		}
		mPeerManager->server_connected();
	}
}

void Museek::cb_trusted_users(const StringList& users) {
	{
		mAllowUploads.clear();
		StringList::const_iterator it, end = users.end();
		for(it = users.begin(); it != end; ++it)
			mAllowUploads[*it] = true;
	}
	{
		map<string, Peer*>& peers = mPeerManager->peers();
		map<string, Peer*>::iterator it, end = peers.end();
		for(it = peers.begin(); it != end; ++it)
			(*it).second->set_trusted(is_trusted((*it).first));
	}
}

void Museek::cb_server_privileged_users(const StringList& users) {
	{
		mPrivileged.clear();
		StringList::const_iterator it, end = users.end();
		for(it = users.begin(); it != end; ++it)
			mPrivileged[*it] = true;
	}
	
	{
		map<string, Peer*>& peers = mPeerManager->peers();
		map<string, Peer*>::iterator it, end = peers.end();
		for(it = peers.begin(); it != end; ++it)
			(*it).second->set_privileged(is_privileged((*it).first));
	}
	msg_server_privileged_users( users.size() );
	
}

void Museek::msg_server_privileged_users( uint32 size ) {}

void Museek::cb_server_add_privileged_user(const string& user) {
	mPrivileged[user] = true;
	Peer* peer = mPeerManager->get_peer(user, false);
	if(peer)
		peer->set_privileged(true);
}

void Museek::cb_server_privileges_left(uint32 time_left) {
}

void Museek::cb_server_parent_inactivity_timeout(uint32 timeout) {
	mParentInactivityTimeout = timeout;
}

void Museek::cb_server_search_inactivity_timeout(uint32 timeout) {
	mSearchInactivityTimeout = timeout;
}

void Museek::cb_server_min_parents_in_cache(uint32 min_parents) {
	mMinParentsInCache = min_parents;
	mDistribManager->check_cache();
}

void Museek::cb_server_distrib_alive_interval(uint32 interval) {
	mDistribAliveInterval = interval;
}

void Museek::cb_server_wishlist_interval(uint32 interval) {
	mWishListInterval = interval;
}

void Museek::cb_server_net_info(const NetInfo& net_info) {
	mDistribManager->add_parents(net_info);
}

void Museek::cb_server_peer_address(const string& user, const string& ip, uint32 port) {
	mPeerManager->set_peer_address(user, ip, port);
}

void Museek::cb_server_peer_exists(const string& user, bool exists) {
	mPeerManager->set_peer_exists(user, exists);
}

void Museek::cb_server_peer_status(const string& user, uint32 status) {
	mPeerManager->set_peer_status(user, status);
}

void Museek::cb_server_peer_stats(const string& user, uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs) {
	mPeerManager->set_peer_stats(user, avgspeed, downloadnum, files, dirs);
}

void Museek::cb_server_peer_privileged(const string& user, bool privileged)
{
	mPrivileged[user] = privileged;
	Peer* peer = mPeerManager->get_peer(user, false);
	if(peer)
		peer->set_privileged(is_privileged(user));
}

void Museek::cb_server_request_peer_connect(const string& user, const string& type, const string& ip, uint32 port, uint32 ticket) {
	mPeerManager->request_peer_connect(user, type, ip, port, ticket);
}

void Museek::cb_server_peer_cannot_connect(const string& user, uint32 token) {
	mPeerManager->peer_cannot_connect(user, token);
}

void Museek::cb_server_room_list(const RoomList& roomlist) {
}
void Museek::cb_server_get_recommendations(const string& item, const Recommendations& recommendations) {
}
void Museek::cb_server_get_global_recommendations(const Recommendations& recommendations) {
}
void Museek::cb_server_get_similar_users(const SimilarUsers& similarusers) {
}
void Museek::cb_server_get_item_recommendations(const string& item, const Recommendations& recommendations) {
}
void Museek::cb_server_get_item_similar_users(const string& item, const SimilarUsers& similarusers) {
}
void Museek::cb_server_joined_room(const string& room, const RoomData& data) {
}

void Museek::cb_server_left_room(const string& room) {
}
void Museek::server_add_interest(const string& interest) {
	if(mServer)
		mServer->add_interest(interest);
}
void Museek::server_remove_interest(const string& interest) {
	if(mServer)
		mServer->remove_interest(interest);
}
void Museek::server_add_hated_interest(const string& interest) {
	if(mServer)
		mServer->add_hated_interest(interest);
}
void Museek::server_remove_hated_interest(const string& interest) {
	if(mServer)
		mServer->remove_hated_interest(interest);
}
void Museek::cb_server_add_interest(const string& interest) {

}
void Museek::cb_server_remove_interest(const string& interest) {

}
void Museek::cb_server_add_hated_interest(const string& interest) {

}
void Museek::cb_server_remove_hated_interest(const string& interest) {

}
void Museek::cb_server_say_room(const string& room, const string& user, const wstring& message) {
}

void Museek::cb_server_user_joined_room(const string& room, const string& user, const UserData& data) {
}

void Museek::cb_server_user_left_room(const string& room, const string& user) {
}

void Museek::cb_server_room_tickers(const string& room, const WTickers& tickers) {
}

void Museek::cb_server_add_room_ticker(const string& room, const string& user, const wstring& message) {
}

void Museek::cb_server_del_room_ticker(const string& room, const string& user) {
}

void Museek::cb_server_private(uint32 msg_id, uint32 timestamp, const string& user, const wstring& message) {
}


void Museek::server_get_global_recommendations() {
	if(mServer)
		mServer->get_global_recommendations();
}
void Museek::server_get_recommendations() {
	if(mServer)
		mServer->get_recommendations();
}
void Museek::server_get_similar_users() {
	if(mServer)
		mServer->get_similar_users();
}
void Museek::server_get_item_recommendations(const string& item) {
	if(mServer)
		mServer->get_item_recommendations(item);
}
void Museek::server_get_item_similar_users(const string& item) {
	if(mServer)
		mServer->get_item_similar_users(item);
}

void Museek::cb_server_exact_file_search(const string& user, uint32 ticket, const string& filename, const string& path, off_t filesize, uint32 checksum) {
}

bool Museek::cb_listen_init(int sock, const string& user, const string& type, uint32 token, const queue<unsigned char>& data) {
	CT("cb_listen_init %u, %s, %s, %u, <...> (%d)", sock, user.c_str(), type.c_str(), token, data.size());
	if(type == "P")
		mPeerManager->get_peer(user)->set_socket(sock, token, data);
	else if(type == "F")
		(new TransferPreConnection(mPeerManager->get_peer(user), token))->set_socket(sock, data);
	else 
		return false;
	return true;
}

bool Museek::cb_listen_pierce_firewall(int sock, uint32 token) {
	CT("cb_listen_pierce_firewall %u, %u", sock, token);
	return mDistribManager->pierced_firewall(sock, token) ||
		mPeerManager->pierced_firewall(sock, token);
}

void Museek::cb_peer_shares(const string& user, const WShares& shares) {
}

void Museek::cb_peer_search(PeerConnection* conn, const string& user, uint32 ticket, const wstring& query) {
	if(! is_banned(user)) {
		Folder results;
		if (is_buddied(user) && mBuddySharesHave) {
			mBuddyShares->search(query, results);
			DEBUG("%i results from buddy-shares", results.size());
			if (! results.size() ) {
				mShares->search(query, results);
				DEBUG("%i results from normal-shares ", results.size());
			}
		} else {
			mShares->search(query, results);
			DEBUG("%i results from normal-shares ", results.size());
		}
		if(! results.empty())
		{
			if(! conn)
				conn = mPeerManager->get_peer(user)->connection();
			conn->results(ticket, results);
		}
			
	}
}

void Museek::cb_peer_info(const string& user, const wstring& info, const vector<unsigned char>& pic, uint32 totalupl, uint32 queuelen, bool slotfree) {
}

void Museek::cb_peer_upload_blocked(const string& user) {
}
void Museek::peer_upload_blocked(const string& user) {
	if(mServer && mUserWarnings) {
		mServer->send_private(user, recoder()->wasciify("[Automatic Message] You are not on my Trusted users list, you can't upload files to me.")); }
}
void Museek::cb_peer_banned(const string& user) {
}
void Museek::peer_banned(const string& user) {
	if(mServer && mUserWarnings) {
		if (! mBuddiesOnly) {
			mServer->send_private(user, recoder()->wasciify("[Automatic Message] You are banned, so you cannot access my files."));
			}
		else if  (mBuddiesOnly) { 
			mServer->send_private(user, recoder()->wasciify("[Automatic Message] You are not on my Buddies list, and I only share to my Buddies."));
			}
	}
}

void Museek::cb_peer_sent_buddy_shares(const string& user) {
}
void Museek::cb_peer_sent_normal_shares(const string& user) {
}
void Museek::cb_peer_sent_user_info(const string& user) {
}
void Museek::cb_server_global_message(const string& message) {
}

void Museek::cb_peer_results(uint32 ticket, const string& user, const WFolder& results, uint32 avgspeed, uint32 queuelen, bool slotfree) {
}

void Museek::cb_peer_folder_contents(const string& user, const wstring& dir, const WShares& contents) {
}

void Museek::cb_transfer_update(const Transfer* transfer) {
}

void Museek::cb_transfer_delete(const Transfer* transfer) {
}

void Museek::cb_distrib_ping() {
}

void Museek::cb_server_send_user_speed(const string& user, uint32 speed) {
	if(mServer)
		mServer->send_user_speed(user, speed);
}
void Museek::cb_server_send_upload_speed(uint32 speed) {
	if(mServer)
		mServer->send_upload_speed(speed);
}
void Museek::cb_peer_transfer_finished(const wstring& path, const string& user) {
}

void Museek::cb_distrib_search(const string& user, uint32 ticket, const wstring& query) {
	cb_peer_search(0, user, ticket, query);
}

void Museek::cannot_connect(BaseConnection *conn, const string& user) {
	if(mTransferManager)
		mTransferManager->cannot_connect(conn);
}

void Museek::connected(BaseConnection *conn, const string& user) {
	if(mTransferManager)
		mTransferManager->connected(conn);
}

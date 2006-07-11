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

#ifndef __MUSEEK_HH__
#define __MUSEEK_HH__

#include <string>
#include <queue>
#include <vector>

#define STR const std::string&
#define WSTR const std::wstring&

class ServerConnection;
class Recoder;
class ShareBase;
class PeerManager;
class PeerListener;
class PeerConnection;
class DistribManager;
class TransferManager;
class Transfer;
class BaseConnection;

#include <Museekal/ConnectionManager.hh>

typedef enum _ConnectMode {
	CM_Active,
	CM_Passive
};
typedef enum _ConnectMode ConnectMode;

class Museek : public ConnectionManager {
public:
	// Constructor and initialisation functions
	Museek();
	
	virtual void init();
	virtual void init_recoder();
	virtual void init_shares();
	virtual void init_buddies_shares();
	virtual void init_peer_manager();
	virtual void init_distrib_manager();
	virtual void init_transfer_manager();
	virtual void init_peer_listener();
	
	inline Recoder* recoder() const { return mRecoder; }
	inline ShareBase* shares() const { return mShares; }
	inline ShareBase* buddyshares() const { return mBuddyShares; }
	inline PeerManager* peer_manager() const { return mPeerManager; }
	inline DistribManager* distrib_manager() const { return mDistribManager; }
	inline TransferManager* transfer_manager() const { return mTransferManager; }
	inline PeerListener* peer_listener() const { return mPeerListener; }
	
	
	// Administrative functions
	inline uint32 token() { return ++mToken; }
	uint mListenPort;
	void set_min_port(uint min_port) { mMinPort = min_port; }
	void set_max_port(uint max_port) { mMaxPort = max_port; }
	void set_port_range(uint min, uint max) { mMinPort = min; mMaxPort = max; }
	
	std::string server_host() const { return mServerHost; }
	uint server_port() const { return mServerPort; }
	void set_server(STR host, uint port) { mServerHost = host, mServerPort = port; }
	
	std::string username() const { return mUsername; }
	void set_username(STR username) { mUsername = username; }
	
	std::string password() const { return mPassword; }
	void set_password(STR password) { mPassword = password; }
	
	inline bool connected() const { return mConnected; }
	inline bool logged_in() const { return mLoggedIn; }

	uint32 parent_inactivity_timeout() const { return mParentInactivityTimeout; }
	uint32 search_inactivity_timeout() const { return mSearchInactivityTimeout; }
	uint32 min_parents_in_cache() const { return mMinParentsInCache; }
	uint32 distrib_alive_interval() const { return mDistribAliveInterval; }
	uint32 wishlist_interval() const { return mWishListInterval; }
	
	std::wstring userinfo() const { return mUserinfo; }
	void set_userinfo(WSTR userinfo) { mUserinfo = userinfo; }
	
	std::vector<unsigned char> userpic() const { return mUserpic; }
	void set_userpic(const std::vector<unsigned char>& userpic) { mUserpic = userpic; }
	
	inline ConnectMode connect_mode() const { return mConnectMode; }
	inline void set_connect_mode(ConnectMode connect_mode) { mConnectMode = connect_mode; }
	
	virtual bool is_privileged(const std::string& user);
	virtual bool is_receiving_shares(const std::string& user)  const;
	virtual bool is_buddied(const std::string& user)  const;
	virtual bool is_banned(const std::string& user) const;
	virtual bool is_trusted(const std::string& user) const;

	inline bool have_buddy_shares() const { return mBuddySharesHave; }
	void set_have_buddy_shares(bool have) { mBuddySharesHave = have; }
	inline bool buddies_only() const { return mBuddiesOnly; }
	void mu_set_only_buddies(bool uw);
	void mu_set_user_warnings(bool only);

	inline uint32 upload_slots() const { return mUploadSlots; }
	inline void set_upload_slots(uint32 n) { mUploadSlots = n; }

	
	// Server related functions
	void server_connect(uint timeout = 60);
	void server_disconnect();
	
	void server_log_in(STR username, STR password);
	void server_shared_folders_files(uint32 folders, uint32 files);
	void server_set_status(uint32 status);
	
	void server_check_privileges();
	
	void server_peer_subscribe(STR user);
	void server_get_peer_address(STR user);
	void server_get_peer_status(STR user);
	void server_get_peer_stats(STR user);
	void server_get_peer_privileged(STR user);
	void server_request_peer_connect(uint32 token, STR user, STR type);
	void server_peer_cannot_connect(uint32 token, STR user);
	
	void server_room_list();
	void server_get_global_recommendations();
	void server_get_recommendations();
	void server_get_similar_users();
	void server_get_item_recommendations(STR item);
	void server_get_item_similar_users(STR item);
	void server_add_interest(STR interest);
	void server_remove_interest(STR interest);
	void server_add_hated_interest(STR interest);
	void server_remove_hated_interest(STR interest);

	void server_join_room(STR room);
	void server_leave_room(STR room);
	void server_say_room(STR room, WSTR message);
	void server_set_ticker(STR room, WSTR message);
	
	void server_send_private(STR user, WSTR message);
	void server_ack_private(uint32 msg_id);
	
	void server_search(uint32 ticket, WSTR query);
	void server_room_search( STR room, uint32 ticket, WSTR query );
	void server_user_search(STR user, uint32 ticket, WSTR query);
	void server_wishlist_search(uint32 ticket, WSTR query);	
	void server_no_parent(bool have_parent);
	
	void server_give_privileges(STR user, uint32 days);
	
	
	// Server related callbacks
	virtual void cb_server_cannot_resolve();
	virtual void cb_server_cannot_connect();
	virtual void cb_server_cannot_login();
	virtual void cb_server_disconnected();
	virtual void cb_server_kicked();
	virtual void cb_server_connected();
	
	virtual void cb_server_logged_in(bool success, STR message);

	virtual void cb_server_privileged_users(const StringList& users);
	virtual void cb_trusted_users(const StringList& users);
	virtual void cb_server_add_privileged_user(STR user);
	virtual void cb_server_privileges_left(uint32 time_left);

	virtual void cb_server_parent_inactivity_timeout(uint32 timeout);
	virtual void cb_server_search_inactivity_timeout(uint32 timeout);
	virtual void cb_server_min_parents_in_cache(uint32 min_parents);
	virtual void cb_server_distrib_alive_interval(uint32 interval);
	virtual void cb_server_wishlist_interval(uint32 interval);
	virtual void cb_server_net_info(const NetInfo& net_info);

	virtual void cb_server_peer_address(STR user, STR ip, uint32 port);
	virtual void cb_server_peer_exists(STR user, bool exists);
	virtual void cb_server_peer_status(STR user, uint32 status);
	virtual void cb_server_peer_stats(STR user, uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs);
	virtual void cb_server_peer_privileged(STR user, bool privileged);
	virtual void cb_server_request_peer_connect(STR user, STR type, STR ip, uint32 port, uint32 token);
	virtual void cb_server_peer_cannot_connect(STR user, uint32 token);
	
	virtual void cb_server_room_list(const RoomList& roomlist);
	virtual void cb_server_get_global_recommendations(const Recommendations& recommendations);
	virtual void cb_server_get_recommendations(STR item, const Recommendations& recommendations);
	virtual void cb_server_get_similar_users(const SimilarUsers& similarusers);
	virtual void cb_server_get_item_recommendations(STR item, const Recommendations& recommendations);
	virtual void cb_server_get_item_similar_users(STR item, const SimilarUsers& similarusers);

	virtual void cb_server_joined_room(STR room, const RoomData& data);
	virtual void cb_server_left_room(STR room);

	virtual void cb_server_add_hated_interest(STR interest);
	virtual void cb_server_remove_hated_interest(STR interest);
	virtual void cb_server_add_interest(STR interest);
	virtual void cb_server_remove_interest(STR interest);

	virtual void cb_server_send_user_speed(STR user, uint32 speed);

	virtual void cb_server_say_room(STR room, STR user, WSTR message);
	virtual void cb_server_user_joined_room(STR room, STR user, const UserData& data);
	virtual void cb_server_user_left_room(STR room, STR user);
	virtual void cb_server_room_tickers(STR room, const WTickers& tickers);
	virtual void cb_server_add_room_ticker(STR room, STR user, WSTR message);
	virtual void cb_server_del_room_ticker(STR room, STR user);
	
	virtual void cb_server_private(uint32 msg_id, uint32 timestamp, STR user, WSTR message);
	

	
	virtual void cb_server_exact_file_search(STR user, uint32 ticket, STR filename, STR path, off_t filesize, uint32 checksum);
	
	
	// Peer listener callbacks
	virtual void cb_listen_init(int sock, STR user, STR type, uint32 token, const std::queue<unsigned char>& data);
	virtual bool cb_listen_pierce_firewall(int sock, uint32 token);
	
	
	// Peer connection callbacks
	virtual void cannot_connect(BaseConnection *conn, STR user);
	virtual void connected(BaseConnection *conn, STR user);
	
	virtual void cb_peer_shares(STR user, const WShares& shares);
	virtual void cb_peer_search(PeerConnection* conn, STR user, uint32 ticket, WSTR query);
	virtual void cb_peer_info(STR user, WSTR info, const std::vector<unsigned char>& pic, uint32 totalupl, uint32 queuelen, bool slotfree);
	virtual void cb_peer_results(uint32 ticket, STR user, const WFolder& results, uint32 avgspeed, uint32 queuelen, bool slotfree);
	virtual void cb_peer_folder_contents(STR user, WSTR dir, const WShares& contents);

	// Status Messages & Warnings
	virtual void cb_peer_upload_blocked(STR user);
	virtual void cb_peer_banned(STR user);
	virtual void cb_peer_sent_buddy_shares(STR user);
	virtual void cb_peer_sent_normal_shares(STR user);
	virtual void cb_server_global_message(STR message);
	virtual void cb_peer_sent_user_info(STR user);
	virtual void cb_peer_transfer_finished(WSTR path, STR user);
	void peer_banned(STR user);
	void peer_upload_blocked(STR user);

	// Transfer callbacks
	virtual void cb_transfer_update(const Transfer* transfer);
	virtual void cb_transfer_delete(const Transfer* transfer);
	
	
	// Distributed network connection callbacks
	virtual void cb_distrib_ping();
	virtual void cb_distrib_search(STR user, uint32 ticket, WSTR query);
	virtual void add_receiving(const std::string& user) ;
	virtual void remove_receiving(const std::string& user) ;
 	bool mUserWarnings, mBuddySharesHave, mBuddiesOnly;
protected:
	// Cycle callback, purges stuff we don't need / want anymore
	virtual void cycle_callback();
	
	
	Recoder* mRecoder;
	
	uint32 mToken;
	
	std::string mServerHost;
	uint mServerPort;
	
	std::string mUsername, mPassword;
	
	std::wstring mUserinfo;
	std::vector<unsigned char> mUserpic;
	
	ServerConnection* mServer;
	bool mConnected, mLoggedIn;
	
	std::map<std::string, bool> mPrivileged, mAllowUploads;
	
	uint32 mParentInactivityTimeout,
	     mSearchInactivityTimeout,
	     mMinParentsInCache,
	     mDistribAliveInterval,
	     mWishListInterval;
	
	ShareBase* mShares, * mBuddyShares;
	
	ConnectMode mConnectMode;
	PeerManager* mPeerManager;
	
	DistribManager* mDistribManager;
	
	uint32 mUploadSlots;
	
	TransferManager* mTransferManager;
	
	PeerListener* mPeerListener;
	uint mMinPort, mMaxPort;
};

#undef STR
#undef WSTR

#endif // __MUSEEK_HH__

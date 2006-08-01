/* museekd - The Museek daemon
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

#ifndef __MUSEEKD_HH__
#define __MUSEEKD_HH__

#include <Museek/Museek.hh>
#include <Muhelp/Muconf.hh>
#include <museekd/MuseekdTypes.hh>
#include <sstream>
#include <list>

class IfaceListener;
class IfaceConnection;
class Buddies;

class Museekd : public Museek {
public:
	Museekd(const std::string& config, const std::string& version);
	
	bool load_config();
	
	void init();
	void init_recoder();
	void init_transfer_manager();
	void init_iface_listeners();
	void init_buddies();
	
	void load_shares();
	void load_buddy_shares();
	
	void reconnect();
	std::string mVersion;
	bool mKicked, mHaveBuddyShares;
	bool is_privileged(const std::string& user);
	bool is_receiving_shares(const std::string& user) const;
	bool is_buddied(const std::string& user) const;
	bool is_banned(const std::string& user) const;
	bool is_trusted(const std::string& user) const;

	void add_receiving(const std::string&);
	void remove_receiving(const std::string&);

	inline bool privilege_buddies() const { return mPrivilegeBuddies; }
	inline bool trusting_uploads() const { return mTrustedUploads; }
	inline bool only_buddies() const { return mOnlyBuddies; }
	
	inline std::string iface_password() const { return mIfacePassword; }
	inline void set_iface_password(const std::string& password) { mIfacePassword = password; }
	
	void new_iface(IfaceConnection* conn);
	void remove_iface(IfaceConnection* conn);
	
	void cb_iface_ping(IfaceConnection* conn, uint32 id);
	
	std::string challenge();
	void cb_iface_login(IfaceConnection* conn, const std::string& algorithm, const std::string& password);
	void cb_iface_check_privileges(IfaceConnection* conn);
	void cb_iface_connect_server(IfaceConnection* conn);
	void cb_iface_disconnect_server(IfaceConnection* conn);
	void cb_iface_reload_shares(IfaceConnection* conn);

	void cb_iface_set_status(IfaceConnection* conn, uint32 status);
	
	void cb_iface_config_set(IfaceConnection* conn, const std::string& domain, const std::string& key, const std::string& value);
	void cb_iface_config_remove(IfaceConnection* conn, const std::string& domain, const std::string& key);
	void cb_iface_config_set_user_image(IfaceConnection* conn, const std::vector<unsigned char>& data);
	
	void cb_iface_peer_exists(IfaceConnection* conn, const std::string& user);
	void cb_iface_peer_status(IfaceConnection* conn, const std::string& user);
	void cb_iface_peer_stats(IfaceConnection* conn, const std::string& user);
	void cb_iface_peer_address(IfaceConnection* conn, const std::string& user);
	void cb_iface_info(IfaceConnection* conn, const std::string& user);
	void cb_iface_shares(IfaceConnection* conn, const std::string& user);
	void cb_iface_give_privileges(IfaceConnection* conn, const std::string& user, uint32 days);
	
	void cb_iface_room_list(IfaceConnection* conn);
	void cb_iface_get_global_recommendations(IfaceConnection* conn);
	void cb_iface_get_recommendations(IfaceConnection* conn);
	void cb_iface_get_similar_users(IfaceConnection* conn);
	void cb_iface_get_item_recommendations(IfaceConnection* conn, const std::string& item);
	void cb_iface_get_item_similar_users(IfaceConnection* conn, const std::string& item);

	void cb_iface_private(IfaceConnection* conn, const std::string& user, const std::string& message);
	void cb_iface_join_room(IfaceConnection* conn, const std::string& room);
	void cb_iface_leave_room(IfaceConnection* conn, const std::string& room);
	void cb_iface_say_room(IfaceConnection* conn, const std::string& room, const std::string& message);
	void cb_iface_room_ticker_set(IfaceConnection* conn, const std::string& room, const std::string& message);

	void cb_iface_user_search(IfaceConnection* conn, const std::string& user, const std::string& query);
	void cb_iface_wishlist_search(IfaceConnection* conn, const std::string& query);	
	void cb_iface_search(IfaceConnection* conn, uint32 type, const std::string& query);
	void cb_iface_terminate(IfaceConnection* conn, const uint32 ticket);
	
	void cb_iface_transfer_update(IfaceConnection* conn, const std::string& user, const std::string& path);
	void cb_iface_transfer_delete(IfaceConnection* conn, bool upload, const std::string& user, const std::string& path);
	void cb_iface_download_file(IfaceConnection* conn, const std::string& user, const std::string& path);
	void cb_iface_download_file_to(IfaceConnection* conn, const std::string& user, const std::string& path, const std::string& dpath);
	void cb_iface_download_folder(IfaceConnection* conn, const std::string& user, const std::string& folder);
	void cb_iface_transfer_abort(IfaceConnection* conn, bool upload, const std::string& user, const std::string& path);
	void cb_iface_upload_file(IfaceConnection* conn, const std::string& user, const std::string& path);
	void cb_iface_add_hated_interest(IfaceConnection* conn, const std::string& interest);
	void cb_iface_add_interest(IfaceConnection* conn, const std::string& interest);
	void cb_iface_remove_hated_interest(IfaceConnection* conn, const std::string& interest);
	void cb_iface_remove_interest(IfaceConnection* conn, const std::string& interest);
	
	void cb_server_cannot_resolve();
	void cb_server_cannot_connect();
	void cb_server_cannot_login();
	void cb_server_disconnected();
	void do_reconnect();
	void cb_server_kicked();
	void cb_server_logged_in(bool success, const std::string& message);
	
	void cb_server_privileges_left(uint32 time_left);
	
	void cb_server_peer_status(const std::string& user, uint32 status);
	void cb_server_peer_stats(const std::string& user, uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs);
	void cb_server_peer_address(const std::string& user, const std::string& ip, uint32 port);
	
	void cb_server_private(uint32 msg_id, uint32 timestamp, const std::string& user, const std::wstring& message);
	void cb_server_room_list(const RoomList& rooms);
	void cb_server_get_global_recommendations(const Recommendations& recommendations);
	void cb_server_get_recommendations(const std::string& interest, const Recommendations& recommendations);
	void cb_server_get_similar_users(const Recommendations& recommendations);
	void cb_server_get_item_recommendations(const std::string& item, const Recommendations& recommendations);
	void cb_server_get_item_similar_users(const std::string& item, const Recommendations& recommendations);
	void cb_server_joined_room(const std::string& room, const RoomData& users);
	void cb_server_left_room(const std::string& room);
	void cb_server_say_room(const std::string& room, const std::string& user, const std::wstring& message);
	void cb_server_user_joined_room(const std::string& room, const std::string& user, const UserData& userdata);
	void cb_server_user_left_room(const std::string& room, const std::string& user);
	void cb_server_room_tickers(const std::string& room, const WTickers& tickers);
	void cb_server_add_room_ticker(const std::string& room, const std::string& user, const std::wstring& message);
	void cb_server_del_room_ticker(const std::string& room, const std::string& user);
	
	void cb_server_add_hated_interest(const std::string& interest);
	void cb_server_add_interest(const std::string& interest);
	void cb_server_remove_hated_interest(const std::string& interest);
	void cb_server_remove_interest(const std::string& interest);
	// Status Messages
	void cb_peer_banned(const std::string& user);
	void cb_peer_upload_blocked(const std::string& user);
	void cb_peer_transfer_finished(const std::wstring& path, const std::string& user);
	void cb_peer_sent_buddy_shares(const std::string& user);
	void cb_peer_sent_normal_shares(const std::string& user);
	void cb_peer_sent_user_info(const std::string& user);
	void cb_server_global_message(const std::string& message);
	void msg_server_privileged_users(uint32 size);

	void cb_peer_shares(const std::string& user, const WShares& shares);
	void cb_peer_info(const std::string& user, const std::wstring& info, const std::vector<unsigned char>& pic, uint32 totalupl, uint32 queuelen, bool slotfree);
	void cb_peer_results(uint32 ticket, const std::string& user, const WFolder& results, uint32 avgspeed, uint32 queuelen, bool slotfree);
	void cb_peer_folder_contents(const std::string& user, const std::wstring& dir, const WShares& contents);
	
	
	void cb_transfer_update(const Transfer* transfer);
	void cb_transfer_delete(const Transfer* transfer);
	
protected:
	void cycle_callback();
	void set_trusting_uploads(bool);
	void set_privilege_buddies(bool);
	void set_only_buddies(bool);
	void set_user_warnings(bool);
	void update_buddies();
	void ban(const std::string&);
	void unban(const std::string&);
	void trust(const std::string&);
	void untrust(const std::string&);	
// 	std::string int_to_string(uint32 number){}
	inline std::string itos(const int i) {
		std::stringstream s;
		s << i;
		return s.str();
	}

private:
	void save_downloads();
	void load_downloads();
	
	int mReconnectTime;
	
	std::string mConfigFile;
	Muconf mConfig;
	
	uint32 mStatus;
	
	Buddies* mBuddies;
	bool mPrivilegeBuddies, mOnlyBuddies, mTrustedUploads;
	
	std::string mIfacePassword;
	std::vector<IfaceConnection*> mIfaces;
	std::vector<std::string> mBanned, mTrusted, mReceiving;
	
	bool mJoined;
	
	RoomList mRoomList;
	Recommendations mRecommendations;
	SimilarUsers mSimilarUsers;
	std::map<std::string, RoomData> mJoinedRooms;
	std::map<std::string, WTickers> mTickers;
	std::map<std::string, WTickers> mSavedTickers;

	std::map<uint32, IfaceConnection*> mSearches;
	
	std::vector<PrivateMessage> mPrivateMessages;
	
	bool mDontSaveDownloads, mSaveDownloads;
	time_t mLastSave;
	
	bool mPonged;
	time_t mPingTime;
};

#endif // __MUSEEKD_HH__

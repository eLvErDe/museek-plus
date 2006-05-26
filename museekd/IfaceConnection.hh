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

#ifndef __IFACE_CONNECTION_HH__
#define __IFACE_CONNECTION_HH__

#include <Museekal/ListenConnection.hh>
#include <Museekal/ClientConnection.hh>
#include <Museek/MessageConnection.hh>
#include <Muhelp/DirEntry.hh>
#include <Mucipher/mucipher.h>

class Museekd;
class Muconf;
class Transfer;

class IfaceListener : public ListenConnection {
public:
	IfaceListener(Museekd *museekd, const std::string& path);
	
protected:
	bool accept();

	Museekd *mMuseekd;
	uint nPort;
	std::string nPath;
};

class IfaceConnection : public MessageConnection {
public:
	IfaceConnection(Museekd *museekd, int sock);
	inline uint32 mask() const { return mMask; };
	inline std::string get_challenge() { return mChallenge; }
	
	
	// Connection and login goo
	void ping(uint32 id);
	void challenge(uint32 version, const std::string& challenge);
	void login_reply(bool ok, const std::string& message, const std::string& new_challenge);
	
	
	// Server goo
	void server_state(bool logged_in, const std::string& username);
	void privileges_left(uint32 time_left);
	void status_set(uint32 status);
	
	void status_message(bool type, const std::string& message);
	
	// Config goo
	void config_state(const std::map<std::string, StringMap>& config);
	
	void config_set(const std::string& domain, const std::string& key, const std::string& value);
	void config_remove(const std::string& domain, const std::string& key);
	
	
	// Peer goo
	void peer_exists(const std::string& user, bool exists);
	void peer_status(const std::string& user, uint32 status);
	void peer_stats(const std::string& user, uint32, uint32, uint32, uint32);
	void peer_address(const std::string&, const std::string&, uint32);
	void info(const std::string& user, const std::string& info, const std::vector<unsigned char>& pic, uint32 avgspeed, uint32 queuelen, bool slotfree);
	void shares(const std::string& user, const Shares& shares);
	
	
	// Chat goo
	void room_state(const RoomList& roomlist, const std::map<std::string, RoomData>& rooms, const std::map<std::string, Tickers>& tickers);
	void room_list(const RoomList& roomlist);

	void get_global_recommendations(const Recommendations& recommendations);
	void get_recommendations(const Recommendations& recommendations);
	void get_similar_users(const SimilarUsers& similarusers);
	void get_item_recommendations(const std::string& item, const Recommendations& recommendations);
	void get_item_similar_users(const std::string& item, const SimilarUsers& similarusers);

	void add_interest(const std::string& interest);
	void remove_interest(const std::string& interest);
	void add_hated_interest(const std::string& interest);
	void remove_hated_interest(const std::string& interest);

	void private_message(uint32 direction, uint32 timestamp, const std::string& user, const std::string& message);
	
	void joined_room(const std::string& room, const RoomData& users);
	void left_room(const std::string& room);
	void user_joined_room(const std::string& room, const std::string& user, const UserData& data);
	void user_left_room(const std::string& room, const std::string& user);
	void say_room(const std::string& room, const std::string& user, const std::string& line);
	
	void room_tickers(const std::string& room, const Tickers& tickers);
	void room_ticker_set(const std::string& room, const std::string& user, const std::string& message);
	
	
	// Search goo
	void search(const std::string& query, uint32 ticket);
	void results(uint32 ticket, const std::string& user, const Folder& results, uint32 avgspeed, uint32 queuelen, bool slotfree);
	
	
	// Transfer goo
	void transfer_state(const std::vector<Transfer*>& uploads, const std::vector<Transfer*>& downloads);
	
	void transfer_update(const Transfer*);
	void transfer_delete(const Transfer*);
	
	
protected:
	virtual void disconnected();
	virtual void process_message(uint32 code);
	
private:
	Museekd *mMuseekd;
	bool mAuthenticated;
	uint32 mMask;
	std::string mChallenge;
	
	CipherContext mContext;
};

#endif // __IFACE_CONNECTION_H__

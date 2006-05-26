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

#ifndef __SERVERCONNECTION_HH__
#define __SERVERCONNECTION_HH__

#include <Museek/MessageConnection.hh>
#include <string>

class Museek;

class ServerConnection : public MessageConnection {
public:
	ServerConnection(Museek *museek);
	
	void log_in(const std::string& username, const std::string& password);
	void listen_port(uint32 listen_port);
	void shared_folders_files(uint32 folders, uint32 files);
	void send_user_speed(const std::string& user, uint32 speed);
	void no_parent(bool have_no_parent);
	void check_privileges();
	void set_status(uint32 status);
	
	void peer_subscribe(const std::string& user);
	void get_peer_address(const std::string& user);
	void get_peer_status(const std::string& user);
	void get_peer_stats(const std::string& user);
	void get_peer_privileged(const std::string& user);
	void request_peer_connect(uint32 token, const std::string& user, const std::string& type);
	void peer_cannot_connect(uint32 token, const std::string& user);
	
	void room_list();
	void get_global_recommendations();
	void get_recommendations();
	void get_similar_users();
	void get_item_recommendations(const std::string& item);
	void get_item_similar_users(const std::string& item);

	void join_room(const std::string& room);
	void leave_room(const std::string& room);

	void say_room(const std::string& room, const std::wstring& message);
	void set_ticker(const std::string& room, const std::wstring& message);
	
	void send_private(const std::string& user, const std::wstring& message);
	void ack_private(uint32 ticket);

	void add_interest(const std::string& interest);
	void add_hated_interest(const std::string& interest);
	void remove_interest(const std::string& interest);
	void remove_hated_interest(const std::string& interest);

	void have_no_parents(bool);
	
	void search(uint32 ticket, const std::wstring& query);
	void room_search( const std::string& room,  uint32 ticket,  const std::wstring& query);
	void user_search( const std::string& user, uint32 ticket, const std::wstring& query);
	
	void give_privileges(const std::string& user, uint32 days);

protected:
	void connected();
	void disconnected();
	void do_disconnect();
	void cannot_resolve();
	void cannot_connect();
	void cannot_login();
	void process_message(uint32);
	int get_poll_mask();
	void return_poll_mask(int);
	
private:
	Museek *mMuseek;
	time_t mLastTraffic;
};

#endif // __SERVERCONNECTION_HH__

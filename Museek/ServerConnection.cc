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

#include <Museek/ServerConnection.hh>
#include <Museek/ServerMessages.hh>
#include <Museek/Museek.hh>
#include <Museek/Recoder.hh>

#ifdef PARSE
# undef PARSE
#endif
#define PARSE(mtype) mtype s; s.parse_network_packet(message);


#define MULOG_DOMAIN "Museek.SC"
#include <Muhelp/Mulog.hh>


#include <string>
using std::string;
using std::wstring;

ServerConnection::ServerConnection(Museek* museek) 
                      : MessageConnection(4), mMuseek(museek) {
	CT("ServerConnection");
}

void ServerConnection::connected() {
	CT("connected");
	
	mMuseek->cb_server_connected();
}

void ServerConnection::cannot_connect() {
	CT("cannot_connect");
	
	mMuseek->cb_server_cannot_connect();
}

void ServerConnection::cannot_resolve() {
	CT("cannot_resolve");
	
	mMuseek->cb_server_cannot_connect();
}

void ServerConnection::disconnected() {
	CT("disconnected");
	
	mMuseek->cb_server_disconnected();
}

void ServerConnection::cannot_login() {
	CT("cannot_login");
	
	mMuseek->cb_server_cannot_login();
}
void ServerConnection::do_disconnect() {
	CT("manual disconnection");
	disconnect();
	mMuseek->cb_server_disconnected();
}

int ServerConnection::get_poll_mask() {
	CT("get_poll_mask");
	
	time_t curtime = time(NULL);
	if(mMuseek->logged_in()) {
		if(curtime >= mLastTraffic + 45) {
			CT("ping");
			SPing s;
			send(s);
			mLastTraffic = curtime;
		}
		next_event = mLastTraffic + 45;
	} else if(ready && curtime >= mLastTraffic + 90) {
		cannot_login();
		disconnect();
		return 0;
	}
	return MessageConnection::get_poll_mask();
}

void ServerConnection::return_poll_mask(int mask) {
	CT("return_poll_mask");
	
	mLastTraffic = time(0);
	next_event = mLastTraffic + 60;
	
	MessageConnection::return_poll_mask(mask);
}

void ServerConnection::process_message(uint32 code) {
	CT("process_message");
	
	switch(code) {
	case 1: {
		PARSE(SLogin)
		DEBUG("login %d, %s", s.success, s.greet.c_str());
		mMuseek->cb_server_logged_in(s.success, s.greet);
		break;
	}
	case 3: {
		PARSE(SGetPeerAddress)
		DEBUG("got peer address %s, %s, %u", s.user.c_str(), s.ip.c_str(), s.port);
		if (mMuseek->username().c_str() == s.user.c_str()) {
			DEBUG("Got your IP address");
			mMuseek->cb_server_peer_address(s.user, "127.0.0.1", s.port);}
		else {
		mMuseek->cb_server_peer_address(s.user, s.ip, s.port);
		}
		break;
	}
	case 5: {
		PARSE(SAddUser)
		DEBUG("subscribed to user %s, %i", s.user.c_str(), s.exists);
		mMuseek->cb_server_peer_exists(s.user, s.exists);
		break;
	}
	case 7: {
		PARSE(SGetStatus)
		DEBUG("got user status %s, %i", s.user.c_str(), s.status);
		mMuseek->cb_server_peer_status(s.user, s.status);
		break;
	}
	case 13: {
		PARSE(SSayChatroom)
		DEBUG("say chat room %s, %s, %s", s.room.c_str(), s.user.c_str(), s.line.c_str());
		mMuseek->cb_server_say_room(s.room, s.user, mMuseek->recoder()->decode_room(s.room, s.line));
		break;
	}
	case 14: {
		PARSE(SJoinRoom)
		DEBUG("joined room %s, %i", s.room.c_str(), s.users.size());
		mMuseek->cb_server_joined_room(s.room, s.users);
		break;
	}
	case 15: {
		PARSE(SLeaveRoom)
		DEBUG("left room %s", s.value.c_str());
		mMuseek->cb_server_left_room(s.value);
		break;
	}
	case 16: {
		PARSE(SUserJoinedRoom)
		DEBUG("user joined room %s, %s", s.room.c_str(), s.user.c_str());
		mMuseek->cb_server_user_joined_room(s.room, s.user, s.userdata);
		break;
	}
	case 17: {
		PARSE(SUserLeftRoom)
		DEBUG("user left room %s, %s", s.room.c_str(), s.user.c_str());
		mMuseek->cb_server_user_left_room(s.room, s.user);
		break;
	}
	case 18: {
		PARSE(SConnectToPeer)
		DEBUG("connect to peer request %s %s %s %u %u", s.user.c_str(), s.type.c_str(), s.ip.c_str(), s.port, s.token);
		mMuseek->cb_server_request_peer_connect(s.user, s.type, s.ip, s.port, s.token);
		break;
	}
	case 22: {
		PARSE(SPrivateMessage)
		DEBUG("got private message %u, %u, %s, %s", s.ticket, s.timestamp, s.user.c_str(), s.message.c_str());
		mMuseek->cb_server_private(s.ticket, s.timestamp, s.user, mMuseek->recoder()->decode_user(s.user, s.message));
		break;
	}
	case 26: {
		PARSE(SFileSearch)
		DEBUG("got file search, %s, %s, %u", s.user.c_str(), s.query.c_str(), s.ticket);
		mMuseek->cb_distrib_search( s.user, s.ticket, mMuseek->recoder()->decode_user(s.user, s.query));
		break;
	}
	case 32:
		CT("pong!");
		break;
	case 36: {
		PARSE(SGetUserStats)
		DEBUG("got user stats %s, %i, %i, %i, %i", s.user.c_str(), s.avgspeed, s.downloadnum, s.files, s.dirs);
		mMuseek->cb_server_peer_stats(s.user.c_str(), s.avgspeed, s.downloadnum, s.files, s.dirs);
		break;
	}
	case 41: {
		PARSE(SKicked)
		DEBUG("Another user logging in with your username, got KICKED from server.");
		mMuseek->cb_server_kicked();
		break;
	}
	case 42: {
		PARSE(SUserSearch)
		DEBUG("User search recieved, %s, %s, %u", s.user.c_str(), s.query.c_str(), s.ticket);
		mMuseek->cb_distrib_search( s.user, s.ticket, mMuseek->recoder()->decode_user(s.user, s.query));
		break;
	}
	case 51: {
		PARSE(SInterestAdd);
		DEBUG("Interest Add %s", s.value.c_str());
		mMuseek->cb_server_add_interest(s.value);
		//   AddThingILike
		break;
	}
	case 52: {
		PARSE(SInterestRemove);
		DEBUG("Interest Remove %s", s.value.c_str());
		mMuseek->cb_server_remove_interest(s.value);
		// RemoveThingILike
		break;
	}
	case 54: {
		PARSE(SGetRecommendations);
		DEBUG("got recommendations <...> (%d)", s.recommendations.size());
		mMuseek->cb_server_get_recommendations("", s.recommendations);
		break;
	}
	case 56: {
		PARSE(SGetGlobalRecommendations);
		DEBUG("got global recommendations <...> (%d)", s.recommendations.size());
		mMuseek->cb_server_get_global_recommendations(s.recommendations);
		break;
	}
	case 64: {
		PARSE(SRoomList)
		DEBUG("got room list, <...> (%d)", s.roomlist.size());
		mMuseek->cb_server_room_list(s.roomlist);
		break;
	}
	case 65: {
		PARSE(SExactFileSearch)
		DEBUG("got exact file search, %s, %u, %s, %s, %lli, %u", s.user.c_str(), s.ticket, s.filename.c_str(), s.path.c_str(), s.filesize, s.checksum);
		mMuseek->cb_server_exact_file_search(s.user, s.ticket, s.filename, s.path, s.filesize, s.checksum);
		break;
	}
	case 66: {
		PARSE(SGlobalMessage)
		DEBUG("got global message, %s", s.value.c_str());
		mMuseek->cb_server_global_message(s.value);
		break;
	}
	case 69: {
		PARSE(SPrivilegedUsers)
		DEBUG("got privileged user list, <...> (%d)", s.values.size());
		mMuseek->cb_server_privileged_users(s.values);
		break;
	}
	case 83:
	case 84:
		break;
	case 86: {
		PARSE(SParentInactivityTimeout)
		DEBUG("parent inactivity timeout %u", s.value);
		mMuseek->cb_server_parent_inactivity_timeout(s.value);
		break;
	}
	case 87: {
		PARSE(SSearchInactivityTimeout)
		DEBUG("search inactivity timeout %u", s.value);
		mMuseek->cb_server_search_inactivity_timeout(s.value);
		break;
	}
	case 88: {
		PARSE(SMinParentsInCache)
		DEBUG("minimum parents in cache %u", s.value);
		mMuseek->cb_server_min_parents_in_cache(s.value);
		break;
	}
	case 90: {
		PARSE(SDistribAliveInterval)
		DEBUG("distrib alive interval %u", s.value);
		mMuseek->cb_server_distrib_alive_interval(s.value);
		break;
	}
	case 91: {
		PARSE(SAddPrivileged)
		DEBUG("add privileged user %s", s.value.c_str());
		mMuseek->cb_server_add_privileged_user(s.value);
		break;
	}
	case 92: {
		PARSE(SCheckPrivileges)
		DEBUG("privileged second left: %i", s.time_left);
		mMuseek->cb_server_privileges_left(s.time_left);
		break;
	}
	case 102: {
		PARSE(SNetInfo)
		DEBUG("NetInfo <...> (%d)", s.users.size());
		mMuseek->cb_server_net_info(s.users);
		break;
	}
	case 104: {
		PARSE(SWishlistInterval)
		DEBUG("wishlist interval %u", s.value);
		mMuseek->cb_server_wishlist_interval(s.value);
		break;
	}
	case 110: {
		PARSE(SGetSimilarUsers)
		DEBUG("got similar users <...> (%d)", s.users.size());
		mMuseek->cb_server_get_similar_users( s.users);
		break;
	}
	case 111: {
		PARSE(SGetItemRecommendations);
		DEBUG("got item recommendations %s, <...> (%d)", s.item.c_str(), s.recommendations.size());
		mMuseek->cb_server_get_item_recommendations(s.item, s.recommendations);
		break;
	}
	case 112: {
		PARSE(SGetItemSimilarUsers);
		DEBUG("got item similar users %s, <...> (%d)", s.item.c_str(), s.users.size());
		mMuseek->cb_server_get_item_similar_users(s.item, s.users);
		break;
	}
	case 113: {
		PARSE(SRoomTickers);
		DEBUG("got room tickers %s, <...> (%d)", s.room.c_str(), s.tickers.size());
		mMuseek->cb_server_room_tickers(s.room, mMuseek->recoder()->decode_tickers(s.room, s.tickers));
		break;
	}
	case 114: {
		PARSE(SRoomTickerAdd);
		DEBUG("got add room ticker %s, %s, %s", s.room.c_str(), s.user.c_str(), s.ticker.c_str());
		mMuseek->cb_server_add_room_ticker(s.room, s.user, mMuseek->recoder()->decode_room(s.room, s.ticker));
		break;
	}
	case 115: {
		PARSE(SRoomTickerRemove);
		DEBUG("got remove room ticker %s, %s", s.room.c_str(), s.user.c_str());
		mMuseek->cb_server_del_room_ticker(s.room, s.user);
		break;
	}
	case 117: {
		PARSE(SInterestHatedAdd);
		DEBUG("Interest Hate Add %s", s.value.c_str());
		mMuseek->cb_server_add_hated_interest(s.value);
		// AddThingIHate
		break;
	}
	case 118: {
		PARSE(SInterestHatedRemove);
		DEBUG("Interest Hate Remove %s", s.value.c_str());
		mMuseek->cb_server_remove_hated_interest(s.value);
		// RemoveThingIHate
		break;
	}
	case 120: {
		PARSE(SRoomSearch)
		DEBUG("Room search recieved, %s, %s, %u", s.user.c_str(), s.query.c_str(), s.ticket);
		mMuseek->cb_distrib_search( s.user, s.ticket, mMuseek->recoder()->decode_user(s.user, s.query));
		break;
	}
	case 122: {
		PARSE(SUserPrivileges);
		DEBUG("got user privileges status %s, %u", s.user.c_str(), s.privileged);
		mMuseek->cb_server_peer_privileged(s.user, s.privileged);
		break;
	}
	case 1001: {
		PARSE(SCannotConnect);
		DEBUG("got cannot connect %s, %u", s.user.c_str(), s.token);
		mMuseek->cb_server_peer_cannot_connect(s.user, s.token);
		break;
	}
	default:
		MessageConnection::process_message(code);
	};
}

void ServerConnection::log_in(const std::string& _u, const std::string& _p) {
	CT("log_in %s, <password>", _u.c_str(), _p.c_str());
	
	SLogin s(_u, _p);
	send(s);
}

void ServerConnection::get_peer_address(const string& _u) {
	CT("get_peer_address %s", _u.c_str());
	
	SGetPeerAddress s(_u);
	send(s);
}

void ServerConnection::request_peer_connect(uint32 _tok, const string& _u, const string& _t) {
	CT("request_peer_connect %u, %s, %s", _tok, _u.c_str(), _t.c_str());
	
	SConnectToPeer s(_tok, _u, _t);
	send(s);
}

void ServerConnection::peer_subscribe(const string& _u) {
	CT("peer_subscribe %s", _u.c_str());
	
	SAddUser s(_u);
	send(s);
}

void ServerConnection::join_room(const string& _r) {
	CT("join_room %s", _r.c_str());
	
	SJoinRoom s(_r);
	send(s);
}

void ServerConnection::leave_room(const string& _r) {
	CT("leave_room %s", _r.c_str());
	
	SLeaveRoom s(_r);
	send(s);
}

void ServerConnection::add_interest(const string& _i) {
	CT("add_interest %s", _i.c_str());
	
	SInterestAdd s(_i);
	send(s);
}
void ServerConnection::add_hated_interest(const string& _i) {
	CT("add_hated_interest %s", _i.c_str());
	
	SInterestHatedAdd s(_i);
	send(s);
}
void ServerConnection::remove_interest(const string& _i) {
	CT("remove_interest %s", _i.c_str());
	
	SInterestRemove s(_i);
	send(s);
}
void ServerConnection::remove_hated_interest(const string& _i) {
	CT("remove_hated_interest %s", _i.c_str());
	
	SInterestHatedRemove s(_i);
	send(s);
}

void ServerConnection::search(uint32 _t, const wstring& _q) {
	CT("search %u, %s", _t, _q.c_str());
	
	SFileSearch s(_t, mMuseek->recoder()->encode_network(_q));
	send(s);
}

void ServerConnection::room_search(const string& _r, uint32 _t,  const wstring& _q) {
	DEBUG("room_search %s: %u, %s", _r.c_str(), _t, mMuseek->recoder()->encode_network(_q).c_str());
	
	SRoomSearch s(_r, _t, mMuseek->recoder()->encode_network(_q));
	send(s);
}

void ServerConnection::user_search(const string& _u, uint32 _t, const wstring& _q) {
	CT("user_search %s: %u, %s", _u.c_str(), _t, _q.c_str());
	
	SUserSearch s(_u, _t, mMuseek->recoder()->encode_network(_q));
	send(s);
}

void ServerConnection::wishlist_search(uint32 _t, const wstring& _q) {
	CT("wishlist_search  %u, %s", _t, _q.c_str());
	
	SWishlistSearch s(_t, mMuseek->recoder()->encode_network(_q));
	send(s);
}

void ServerConnection::room_list() {
	CT("room_list");
	
	SRoomList s;
	send(s);
}

void ServerConnection::get_global_recommendations() {
	CT("get_global_recommendations");
	
	SGetGlobalRecommendations s;
	send(s);
}

void ServerConnection::get_recommendations() {
	CT("get_recommendations");
	
	SGetRecommendations s;
	send(s);
}
void ServerConnection::get_similar_users() {
	CT("get_similar_users");
	
	SGetSimilarUsers s;
	send(s);
}
void ServerConnection::get_item_similar_users(const string& _i) {
	CT("get_item_similar_users");
	
	SGetItemSimilarUsers s(_i);
	send(s);
}
void ServerConnection::get_item_recommendations(const string& _i) {
	CT("get_item_recommendations");
	
	SGetItemRecommendations s(_i);
	send(s);
}
void ServerConnection::say_room(const string& _r, const wstring& _m) {
	CT("say_room %s, XXX", _r.c_str());
	
	SSayChatroom s(_r, mMuseek->recoder()->encode_room(_r, _m));
	send(s);
}

void ServerConnection::set_ticker(const string& _r, const wstring& _m) {
	CT("set_ticker %s XXX", _r.c_str());
	
	SSetRoomTicker s(_r, mMuseek->recoder()->encode_room(_r, _m));
	send(s);
}

void ServerConnection::send_private(const string& _u, const wstring& _m) {
	CT("send_private %s, %s", _u.c_str(), _m.c_str());
	
	SPrivateMessage s(_u, mMuseek->recoder()->encode_user(_u, _m));
	send(s);
}

void ServerConnection::ack_private(uint32 _t) {
	CT("ack_private %u", _t);
	
	SAckPrivateMessage s(_t);
	send(s);
}

void ServerConnection::shared_folders_files(uint32 _d, uint32 _f) {
	CT("shared_folders_files %u %u", _d, _f);
	
	SSharedFoldersFiles s(_d, _f);
	send(s);
}

void ServerConnection::send_user_speed(const string& _u, uint32 _s) {
	// Depreciated on SoulSeek
	CT("send_user_speed %s %u", _u.c_str(), _s);
	
	SSendSpeed s(_u, _s);
	send(s);
}

void ServerConnection::send_upload_speed(uint32 _s) {
	CT("send_upload_speed %u", _s);
	
	SSendUploadSpeed s(_s);
	send(s);
}

void ServerConnection::get_peer_status(const string& _u) {
	CT("get_peer_status %s", _u.c_str());
	
	SGetStatus s(_u);
	send(s);
}

void ServerConnection::no_parent(bool _p) {
	CT("no_parent %d", _p);
	
	SHaveNoParents s(_p);
	send(s);
}

void ServerConnection::get_peer_stats(const string& _u) {
	CT("do_get_peer_stats %s", _u.c_str());
	
	SGetUserStats s(_u);
	send(s);
}

void ServerConnection::peer_cannot_connect(uint32 _t, const string& _u) {
	CT("peer_cannot_connect %s, %u", _u.c_str(), _t);
	
	SCannotConnect s(_u, _t);
	send(s);
}

void ServerConnection::listen_port(uint32 _p) {
	CT("listen_port %u", _p);
	
	SSetListenPort s(_p);
	send(s);
}

void ServerConnection::have_no_parents(bool b) {
	CT("have_no_parents %i", b);
	
	SHaveNoParents s(b);
	send(s);
}

void ServerConnection::check_privileges() {
	CT("check_privileges");
	
	SCheckPrivileges s;
	send(s);
}

void ServerConnection::set_status(uint32 _s) {
	CT("set_status %i", _s);
	
	SSetStatus s(_s);
	send(s);
}

void ServerConnection::get_peer_privileged(const std::string& _u) {
	CT("get_peer_privileged %s", _u.c_str());
	
	SUserPrivileges s(_u);
	send(s);
}

void ServerConnection::give_privileges(const std::string& _u, uint32 _d) {
	CT("give_privileges %s, %i", _u.c_str(), _d);
	
	SGivePrivileges s(_u, _d);
	send(s);
}
